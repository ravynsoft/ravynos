/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_defines.h"
#include "util/format/u_format.h"

#include "freedreno_util.h"

int32_t marker_cnt;

enum adreno_rb_depth_format
fd_pipe2depth(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      return DEPTHX_16;
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      return DEPTHX_24_8;
   case PIPE_FORMAT_Z32_FLOAT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return DEPTHX_32;
   default:
      return ~0;
   }
}

enum pc_di_index_size
fd_pipe2index(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_R8_UINT:
      return INDEX_SIZE_8_BIT;
   case PIPE_FORMAT_R16_UINT:
      return INDEX_SIZE_16_BIT;
   case PIPE_FORMAT_R32_UINT:
      return INDEX_SIZE_32_BIT;
   default:
      return ~0;
   }
}

/* we need to special case a bit the depth/stencil restore, because we are
 * using the texture sampler to blit into the depth/stencil buffer, *not*
 * into a color buffer.  Otherwise fdN_tex_swiz() will do the wrong thing,
 * as it is assuming that you are sampling into normal render target..
 */
enum pipe_format
fd_gmem_restore_format(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      return PIPE_FORMAT_R8G8B8A8_UNORM;
   case PIPE_FORMAT_Z16_UNORM:
      return PIPE_FORMAT_R8G8_UNORM;
   case PIPE_FORMAT_S8_UINT:
      return PIPE_FORMAT_R8_UNORM;
   default:
      return format;
   }
}

enum adreno_rb_blend_factor
fd_blend_factor(unsigned factor)
{
   switch (factor) {
   case PIPE_BLENDFACTOR_ONE:
      return FACTOR_ONE;
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return FACTOR_SRC_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return FACTOR_SRC_ALPHA;
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return FACTOR_DST_ALPHA;
   case PIPE_BLENDFACTOR_DST_COLOR:
      return FACTOR_DST_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return FACTOR_SRC_ALPHA_SATURATE;
   case PIPE_BLENDFACTOR_CONST_COLOR:
      return FACTOR_CONSTANT_COLOR;
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      return FACTOR_CONSTANT_ALPHA;
   case PIPE_BLENDFACTOR_ZERO:
   case 0:
      return FACTOR_ZERO;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      return FACTOR_ONE_MINUS_SRC_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      return FACTOR_ONE_MINUS_SRC_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      return FACTOR_ONE_MINUS_DST_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
      return FACTOR_ONE_MINUS_DST_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
      return FACTOR_ONE_MINUS_CONSTANT_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
      return FACTOR_ONE_MINUS_CONSTANT_ALPHA;
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
      return FACTOR_ONE_MINUS_SRC1_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
      return FACTOR_ONE_MINUS_SRC1_ALPHA;
   case PIPE_BLENDFACTOR_SRC1_COLOR:
      return FACTOR_SRC1_COLOR;
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
      return FACTOR_SRC1_ALPHA;
   default:
      DBG("invalid blend factor: %x", factor);
      return 0;
   }
}

enum adreno_pa_su_sc_draw
fd_polygon_mode(unsigned mode)
{
   switch (mode) {
   case PIPE_POLYGON_MODE_POINT:
      return PC_DRAW_POINTS;
   case PIPE_POLYGON_MODE_LINE:
      return PC_DRAW_LINES;
   case PIPE_POLYGON_MODE_FILL:
      return PC_DRAW_TRIANGLES;
   default:
      DBG("invalid polygon mode: %u", mode);
      return 0;
   }
}

enum adreno_stencil_op
fd_stencil_op(unsigned op)
{
   switch (op) {
   case PIPE_STENCIL_OP_KEEP:
      return STENCIL_KEEP;
   case PIPE_STENCIL_OP_ZERO:
      return STENCIL_ZERO;
   case PIPE_STENCIL_OP_REPLACE:
      return STENCIL_REPLACE;
   case PIPE_STENCIL_OP_INCR:
      return STENCIL_INCR_CLAMP;
   case PIPE_STENCIL_OP_DECR:
      return STENCIL_DECR_CLAMP;
   case PIPE_STENCIL_OP_INCR_WRAP:
      return STENCIL_INCR_WRAP;
   case PIPE_STENCIL_OP_DECR_WRAP:
      return STENCIL_DECR_WRAP;
   case PIPE_STENCIL_OP_INVERT:
      return STENCIL_INVERT;
   default:
      DBG("invalid stencil op: %u", op);
      return 0;
   }
}
