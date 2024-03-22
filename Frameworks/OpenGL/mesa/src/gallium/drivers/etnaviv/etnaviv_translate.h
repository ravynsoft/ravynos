/*
 * Copyright (c) 2012-2013 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/* inlined translation functions between gallium and vivante */
#ifndef H_TRANSLATE
#define H_TRANSLATE

#include "pipe/p_defines.h"
#include "util/format/u_formats.h"
#include "pipe/p_state.h"

#include "etnaviv_debug.h"
#include "etnaviv_format.h"
#include "etnaviv_util.h"
#include "hw/cmdstream.xml.h"
#include "hw/common_3d.xml.h"
#include "hw/state.xml.h"
#include "hw/state_3d.xml.h"
#include "hw/state_blt.xml.h"

#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_pack_color.h"

/* Returned when there is no match of pipe value to etna value */
#define ETNA_NO_MATCH (~0)

static inline uint32_t
translate_cull_face(unsigned cull_face, unsigned front_ccw)
{
   switch (cull_face) {
   case PIPE_FACE_NONE:
   case PIPE_FACE_FRONT_AND_BACK: /* handled in draw_vbo */
      return VIVS_PA_CONFIG_CULL_FACE_MODE_OFF;
   case PIPE_FACE_BACK:
      return front_ccw ? VIVS_PA_CONFIG_CULL_FACE_MODE_CW
                       : VIVS_PA_CONFIG_CULL_FACE_MODE_CCW;
   case PIPE_FACE_FRONT:
      return front_ccw ? VIVS_PA_CONFIG_CULL_FACE_MODE_CCW
                       : VIVS_PA_CONFIG_CULL_FACE_MODE_CW;
   default:
      DBG("Unhandled cull face mode %i", cull_face);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_polygon_mode(unsigned polygon_mode)
{
   switch (polygon_mode) {
   case PIPE_POLYGON_MODE_FILL:
      return VIVS_PA_CONFIG_FILL_MODE_SOLID;
   case PIPE_POLYGON_MODE_LINE:
      return VIVS_PA_CONFIG_FILL_MODE_WIREFRAME;
   case PIPE_POLYGON_MODE_POINT:
      return VIVS_PA_CONFIG_FILL_MODE_POINT;
   default:
      DBG("Unhandled polygon mode %i", polygon_mode);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_stencil_mode(bool enable_0, bool enable_1)
{
   if (enable_0) {
      return enable_1 ? VIVS_PE_STENCIL_CONFIG_MODE_TWO_SIDED
                      : VIVS_PE_STENCIL_CONFIG_MODE_ONE_SIDED;
   } else {
      return VIVS_PE_STENCIL_CONFIG_MODE_DISABLED;
   }
}

static inline uint32_t
translate_stencil_op(unsigned stencil_op)
{
   switch (stencil_op) {
   case PIPE_STENCIL_OP_KEEP:
      return STENCIL_OP_KEEP;
   case PIPE_STENCIL_OP_ZERO:
      return STENCIL_OP_ZERO;
   case PIPE_STENCIL_OP_REPLACE:
      return STENCIL_OP_REPLACE;
   case PIPE_STENCIL_OP_INCR:
      return STENCIL_OP_INCR;
   case PIPE_STENCIL_OP_DECR:
      return STENCIL_OP_DECR;
   case PIPE_STENCIL_OP_INCR_WRAP:
      return STENCIL_OP_INCR_WRAP;
   case PIPE_STENCIL_OP_DECR_WRAP:
      return STENCIL_OP_DECR_WRAP;
   case PIPE_STENCIL_OP_INVERT:
      return STENCIL_OP_INVERT;
   default:
      DBG("Unhandled stencil op: %i", stencil_op);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_blend_factor(unsigned blend_factor)
{
   switch (blend_factor) {
   case PIPE_BLENDFACTOR_ONE:
      return BLEND_FUNC_ONE;
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return BLEND_FUNC_SRC_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return BLEND_FUNC_SRC_ALPHA;
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return BLEND_FUNC_DST_ALPHA;
   case PIPE_BLENDFACTOR_DST_COLOR:
      return BLEND_FUNC_DST_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return BLEND_FUNC_SRC_ALPHA_SATURATE;
   case PIPE_BLENDFACTOR_CONST_COLOR:
      return BLEND_FUNC_CONSTANT_COLOR;
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      return BLEND_FUNC_CONSTANT_ALPHA;
   case PIPE_BLENDFACTOR_ZERO:
      return BLEND_FUNC_ZERO;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      return BLEND_FUNC_ONE_MINUS_SRC_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      return BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      return BLEND_FUNC_ONE_MINUS_DST_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
      return BLEND_FUNC_ONE_MINUS_DST_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
      return BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
      return BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA;
   case PIPE_BLENDFACTOR_SRC1_COLOR:
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
   default:
      DBG("Unhandled blend factor: %i", blend_factor);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_texture_wrapmode(unsigned wrap)
{
   switch (wrap) {
   case PIPE_TEX_WRAP_REPEAT:
      return TEXTURE_WRAPMODE_REPEAT;
   case PIPE_TEX_WRAP_CLAMP:
      return TEXTURE_WRAPMODE_CLAMP_TO_EDGE;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return TEXTURE_WRAPMODE_CLAMP_TO_EDGE;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      return TEXTURE_WRAPMODE_CLAMP_TO_EDGE; /* XXX */
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return TEXTURE_WRAPMODE_MIRRORED_REPEAT;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
      return TEXTURE_WRAPMODE_MIRRORED_REPEAT; /* XXX */
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      return TEXTURE_WRAPMODE_MIRRORED_REPEAT; /* XXX */
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      return TEXTURE_WRAPMODE_MIRRORED_REPEAT; /* XXX */
   default:
      DBG("Unhandled texture wrapmode: %i", wrap);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_texture_mipfilter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_MIPFILTER_NEAREST:
      return TEXTURE_FILTER_NEAREST;
   case PIPE_TEX_MIPFILTER_LINEAR:
      return TEXTURE_FILTER_LINEAR;
   case PIPE_TEX_MIPFILTER_NONE:
      return TEXTURE_FILTER_NONE;
   default:
      DBG("Unhandled texture mipfilter: %i", filter);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_texture_filter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_FILTER_NEAREST:
      return TEXTURE_FILTER_NEAREST;
   case PIPE_TEX_FILTER_LINEAR:
      return TEXTURE_FILTER_LINEAR;
   default:
      DBG("Unhandled texture filter: %i", filter);
      return ETNA_NO_MATCH;
   }
}

static inline int
translate_rb_src_dst_swap(enum pipe_format src, enum pipe_format dst)
{
   return translate_pe_format_rb_swap(src) ^ translate_pe_format_rb_swap(dst);
}

static inline uint32_t
translate_depth_format(enum pipe_format fmt)
{
   /* Note: Pipe format convention is LSB to MSB, VIVS is MSB to LSB */
   switch (fmt) {
   case PIPE_FORMAT_Z16_UNORM:
      return VIVS_PE_DEPTH_CONFIG_DEPTH_FORMAT_D16;
   case PIPE_FORMAT_X8Z24_UNORM:
      return VIVS_PE_DEPTH_CONFIG_DEPTH_FORMAT_D24S8;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      return VIVS_PE_DEPTH_CONFIG_DEPTH_FORMAT_D24S8;
   default:
      return ETNA_NO_MATCH;
   }
}

/* render target format for MSAA */
static inline uint32_t
translate_ts_format(enum pipe_format fmt)
{
   /* Note: Pipe format convention is LSB to MSB, VIVS is MSB to LSB */
   switch (fmt) {
   case PIPE_FORMAT_B4G4R4X4_UNORM:
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return COMPRESSION_FORMAT_A4R4G4B4;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
      return COMPRESSION_FORMAT_A1R5G5B5;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return COMPRESSION_FORMAT_A1R5G5B5;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return COMPRESSION_FORMAT_R5G6B5;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_SRGB:
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      return COMPRESSION_FORMAT_X8R8G8B8;
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_SRGB:
   case PIPE_FORMAT_R8G8B8A8_UNORM:
      return COMPRESSION_FORMAT_A8R8G8B8;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      return COMPRESSION_FORMAT_D24S8;
   case PIPE_FORMAT_X8Z24_UNORM:
      return COMPRESSION_FORMAT_D24X8;
   case PIPE_FORMAT_Z16_UNORM:
      return COMPRESSION_FORMAT_D16;
   /* MSAA with YUYV not supported */
   default:
      return ETNA_NO_MATCH;
   }
}

/* formats directly supported in the RS engine */
static inline uint32_t
translate_rs_format(enum pipe_format fmt)
{
   /* Note: Pipe format convention is LSB to MSB, VIVS is MSB to LSB */
   switch (fmt) {
   case PIPE_FORMAT_B4G4R4X4_UNORM:
      return RS_FORMAT_X4R4G4B4;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return RS_FORMAT_A4R4G4B4;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
      return RS_FORMAT_X1R5G5B5;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return RS_FORMAT_A1R5G5B5;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return RS_FORMAT_R5G6B5;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_SRGB:
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      return RS_FORMAT_X8R8G8B8;
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_SRGB:
   case PIPE_FORMAT_R8G8B8A8_UNORM:
      return RS_FORMAT_A8R8G8B8;
   default:
      return ETNA_NO_MATCH;
   }
}

/* formats directly supported in the BLT engine */
static inline uint32_t
translate_blt_format(enum pipe_format fmt)
{
   /* Note: Pipe format convention is LSB to MSB, VIVS is MSB to LSB */
   switch (fmt) {
   case PIPE_FORMAT_B4G4R4X4_UNORM:
      return BLT_FORMAT_X4R4G4B4;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return BLT_FORMAT_A4R4G4B4;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
      return BLT_FORMAT_X1R5G5B5;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return BLT_FORMAT_A1R5G5B5;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return BLT_FORMAT_R5G6B5;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_SRGB:
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      return BLT_FORMAT_X8R8G8B8;
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_SRGB:
   case PIPE_FORMAT_R8G8B8A8_UNORM:
      return BLT_FORMAT_A8R8G8B8;
   case PIPE_FORMAT_R10G10B10A2_UNORM:
   case PIPE_FORMAT_R10G10B10X2_UNORM:
      return BLT_FORMAT_A2R10G10B10;
   default:
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
drmfourcc_to_ts_format(uint32_t fourcc)
{
   switch (fourcc) {
   case DRM_FORMAT_ARGB8888:
      return COMPRESSION_FORMAT_A8R8G8B8;
   case DRM_FORMAT_XRGB8888:
      return COMPRESSION_FORMAT_X8R8G8B8;
   case DRM_FORMAT_RGB565:
      return COMPRESSION_FORMAT_R5G6B5;
   case DRM_FORMAT_ARGB4444:
      return COMPRESSION_FORMAT_A4R4G4B4;
   case DRM_FORMAT_ARGB1555:
      return COMPRESSION_FORMAT_A1R5G5B5;
   default:
      return ~0;
   }
}

static inline uint32_t
ts_format_to_drmfourcc(uint32_t comp_format)
{
   switch (comp_format) {
   case COMPRESSION_FORMAT_A8R8G8B8:
      return DRM_FORMAT_ARGB8888;
   case COMPRESSION_FORMAT_X8R8G8B8:
      return DRM_FORMAT_XRGB8888;
   case COMPRESSION_FORMAT_R5G6B5:
      return DRM_FORMAT_RGB565;
   case COMPRESSION_FORMAT_A4R4G4B4:
      return DRM_FORMAT_ARGB4444;
   case COMPRESSION_FORMAT_A1R5G5B5:
      return DRM_FORMAT_ARGB1555;
   default:
      return 0;
   }
}

/* Return normalization flag for vertex element format */
static inline uint32_t
translate_vertex_format_normalize(enum pipe_format fmt)
{
   const struct util_format_description *desc = util_format_description(fmt);

   /* assumes that normalization of channel 0 holds for all channels;
    * this holds for all vertex formats that we support */
   return desc->channel[0].normalized
             ? VIVS_FE_VERTEX_ELEMENT_CONFIG_NORMALIZE_SIGN_EXTEND
             : VIVS_FE_VERTEX_ELEMENT_CONFIG_NORMALIZE_OFF;
}

static inline uint32_t
translate_output_mode(enum pipe_format fmt, bool halti5)
{
   const unsigned bits =
      util_format_get_component_bits(fmt, UTIL_FORMAT_COLORSPACE_RGB, 0);

   if (bits == 32)
      return COLOR_OUTPUT_MODE_UIF32;

   if (!util_format_is_pure_integer(fmt))
      return COLOR_OUTPUT_MODE_NORMAL;

   /* generic integer output mode pre-halti5 (?) */
   if (bits == 10 || !halti5)
      return COLOR_OUTPUT_MODE_A2B10G10R10UI;

   if (util_format_is_pure_sint(fmt))
      return bits == 8 ? COLOR_OUTPUT_MODE_I8 : COLOR_OUTPUT_MODE_I16;

   return bits == 8 ? COLOR_OUTPUT_MODE_U8 : COLOR_OUTPUT_MODE_U16;
}

static inline uint32_t
translate_index_size(unsigned index_size)
{
   switch (index_size) {
   case 1:
      return VIVS_FE_INDEX_STREAM_CONTROL_TYPE_UNSIGNED_CHAR;
   case 2:
      return VIVS_FE_INDEX_STREAM_CONTROL_TYPE_UNSIGNED_SHORT;
   case 4:
      return VIVS_FE_INDEX_STREAM_CONTROL_TYPE_UNSIGNED_INT;
   default:
      DBG("Unhandled index size %i", index_size);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_draw_mode(unsigned mode)
{
   switch (mode) {
   case MESA_PRIM_POINTS:
      return PRIMITIVE_TYPE_POINTS;
   case MESA_PRIM_LINES:
      return PRIMITIVE_TYPE_LINES;
   case MESA_PRIM_LINE_LOOP:
      return PRIMITIVE_TYPE_LINE_LOOP;
   case MESA_PRIM_LINE_STRIP:
      return PRIMITIVE_TYPE_LINE_STRIP;
   case MESA_PRIM_TRIANGLES:
      return PRIMITIVE_TYPE_TRIANGLES;
   case MESA_PRIM_TRIANGLE_STRIP:
      return PRIMITIVE_TYPE_TRIANGLE_STRIP;
   case MESA_PRIM_TRIANGLE_FAN:
      return PRIMITIVE_TYPE_TRIANGLE_FAN;
   case MESA_PRIM_QUADS:
      return PRIMITIVE_TYPE_QUADS;
   default:
      DBG("Unhandled draw mode primitive %i", mode);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_clear_depth_stencil(enum pipe_format format, double depth,
                              uint8_t stencil)
{
   uint32_t clear_value = util_pack_z_stencil(format, depth, stencil);

   if (format == PIPE_FORMAT_Z16_UNORM)
      clear_value |= clear_value << 16;

   return clear_value;
}

/* Convert MSAA number of samples to x and y scaling factor.
 * Return true if supported and false otherwise. */
static inline bool
translate_samples_to_xyscale(int num_samples, int *xscale_out, int *yscale_out)
{
   int xscale, yscale;

   switch (num_samples) {
   case 0:
   case 1:
      xscale = 1;
      yscale = 1;
      break;
   case 2:
      xscale = 2;
      yscale = 1;
      break;
   case 4:
      xscale = 2;
      yscale = 2;
      break;
   default:
      return false;
   }

   if (xscale_out)
      *xscale_out = xscale;
   if (yscale_out)
      *yscale_out = yscale;

   return true;
}

static inline uint32_t
translate_texture_target(unsigned target)
{
   switch (target) {
   case PIPE_TEXTURE_1D:
      return TEXTURE_TYPE_1D;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_1D_ARRAY:
      return TEXTURE_TYPE_2D;
   case PIPE_TEXTURE_CUBE:
      return TEXTURE_TYPE_CUBE_MAP;
   case PIPE_TEXTURE_3D:
   case PIPE_TEXTURE_2D_ARRAY:
      return TEXTURE_TYPE_3D;
   default:
      DBG("Unhandled texture target: %i", target);
      return ETNA_NO_MATCH;
   }
}

static inline uint32_t
translate_texture_compare(enum pipe_compare_func compare_func)
{
   switch (compare_func) {
   case PIPE_FUNC_NEVER:
      return TEXTURE_COMPARE_FUNC_NEVER;
   case PIPE_FUNC_LESS:
      return TEXTURE_COMPARE_FUNC_LESS;
   case PIPE_FUNC_EQUAL:
      return TEXTURE_COMPARE_FUNC_EQUAL;
   case PIPE_FUNC_LEQUAL:
      return TEXTURE_COMPARE_FUNC_LEQUAL;
   case PIPE_FUNC_GREATER:
      return TEXTURE_COMPARE_FUNC_GREATER;
   case PIPE_FUNC_NOTEQUAL:
      return TEXTURE_COMPARE_FUNC_NOTEQUAL;
   case PIPE_FUNC_GEQUAL:
      return TEXTURE_COMPARE_FUNC_GEQUAL;
   case PIPE_FUNC_ALWAYS:
      return TEXTURE_COMPARE_FUNC_ALWAYS;
   default:
      unreachable("Invalid compare func");
   }
}

#endif
