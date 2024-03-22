/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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

#include "pipe/p_screen.h"
#include "util/format/u_format.h"

#include "fd2_context.h"
#include "fd2_emit.h"
#include "fd2_resource.h"
#include "fd2_screen.h"
#include "fd2_util.h"

static bool
fd2_screen_is_format_supported(struct pipe_screen *pscreen,
                               enum pipe_format format,
                               enum pipe_texture_target target,
                               unsigned sample_count,
                               unsigned storage_sample_count, unsigned usage)
{
   unsigned retval = 0;

   if ((target >= PIPE_MAX_TEXTURE_TYPES) ||
       (sample_count > 1)) { /* TODO add MSAA */
      DBG("not supported: format=%s, target=%d, sample_count=%d, usage=%x",
          util_format_name(format), target, sample_count, usage);
      return false;
   }

   if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
      return false;

   if ((usage & PIPE_BIND_RENDER_TARGET) &&
       fd2_pipe2color(format) != (enum a2xx_colorformatx) ~0) {
      retval |= PIPE_BIND_RENDER_TARGET;
   }

   if ((usage & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_VERTEX_BUFFER)) &&
       !util_format_is_srgb(format) && !util_format_is_pure_integer(format) &&
       fd2_pipe2surface(format).format != FMT_INVALID) {
      retval |= usage & PIPE_BIND_VERTEX_BUFFER;
      /* the only npot blocksize supported texture format is R32G32B32_FLOAT */
      if (util_is_power_of_two_or_zero(util_format_get_blocksize(format)) ||
          format == PIPE_FORMAT_R32G32B32_FLOAT)
         retval |= usage & PIPE_BIND_SAMPLER_VIEW;
   }

   if ((usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET |
                 PIPE_BIND_SCANOUT | PIPE_BIND_SHARED)) &&
       (fd2_pipe2color(format) != (enum a2xx_colorformatx) ~0)) {
      retval |= usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET |
                         PIPE_BIND_SCANOUT | PIPE_BIND_SHARED);
   }

   if ((usage & PIPE_BIND_DEPTH_STENCIL) &&
       (fd_pipe2depth(format) != (enum adreno_rb_depth_format) ~0)) {
      retval |= PIPE_BIND_DEPTH_STENCIL;
   }

   if ((usage & PIPE_BIND_INDEX_BUFFER) &&
       (fd_pipe2index(format) != (enum pc_di_index_size) ~0)) {
      retval |= PIPE_BIND_INDEX_BUFFER;
   }

   if (retval != usage) {
      DBG("not supported: format=%s, target=%d, sample_count=%d, "
          "usage=%x, retval=%x",
          util_format_name(format), target, sample_count, usage, retval);
   }

   return retval == usage;
}

/* clang-format off */
static const enum pc_di_primtype a22x_primtypes[MESA_PRIM_COUNT] = {
   [MESA_PRIM_POINTS]         = DI_PT_POINTLIST_PSIZE,
   [MESA_PRIM_LINES]          = DI_PT_LINELIST,
   [MESA_PRIM_LINE_STRIP]     = DI_PT_LINESTRIP,
   [MESA_PRIM_LINE_LOOP]      = DI_PT_LINELOOP,
   [MESA_PRIM_TRIANGLES]      = DI_PT_TRILIST,
   [MESA_PRIM_TRIANGLE_STRIP] = DI_PT_TRISTRIP,
   [MESA_PRIM_TRIANGLE_FAN]   = DI_PT_TRIFAN,
};

static const enum pc_di_primtype a20x_primtypes[MESA_PRIM_COUNT] = {
   [MESA_PRIM_POINTS]         = DI_PT_POINTLIST_PSIZE,
   [MESA_PRIM_LINES]          = DI_PT_LINELIST,
   [MESA_PRIM_LINE_STRIP]     = DI_PT_LINESTRIP,
   [MESA_PRIM_TRIANGLES]      = DI_PT_TRILIST,
   [MESA_PRIM_TRIANGLE_STRIP] = DI_PT_TRISTRIP,
   [MESA_PRIM_TRIANGLE_FAN]   = DI_PT_TRIFAN,
};
/* clang-format on */

void
fd2_screen_init(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   screen->max_rts = 1;
   pscreen->context_create = fd2_context_create;
   pscreen->is_format_supported = fd2_screen_is_format_supported;

   screen->setup_slices = fd2_setup_slices;
   if (FD_DBG(TTILE))
      screen->tile_mode = fd2_tile_mode;

   fd2_emit_init_screen(pscreen);

   if (screen->gpu_id >= 220) {
      screen->primtypes = a22x_primtypes;
   } else {
      screen->primtypes = a20x_primtypes;
   }
}
