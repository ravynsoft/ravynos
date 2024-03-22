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

#include "fd3_context.h"
#include "fd3_emit.h"
#include "fd3_format.h"
#include "fd3_resource.h"
#include "fd3_screen.h"

#include "ir3/ir3_compiler.h"

static bool
fd3_screen_is_format_supported(struct pipe_screen *pscreen,
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

   if ((usage & PIPE_BIND_VERTEX_BUFFER) &&
       (fd3_pipe2vtx(format) != VFMT_NONE)) {
      retval |= PIPE_BIND_VERTEX_BUFFER;
   }

   if ((usage & PIPE_BIND_SAMPLER_VIEW) &&
       (fd3_pipe2tex(format) != TFMT_NONE)) {
      retval |= PIPE_BIND_SAMPLER_VIEW;
   }

   if ((usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET |
                 PIPE_BIND_SCANOUT | PIPE_BIND_SHARED | PIPE_BIND_BLENDABLE)) &&
       (fd3_pipe2color(format) != RB_NONE) &&
       (fd3_pipe2tex(format) != TFMT_NONE)) {
      retval |= usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET |
                         PIPE_BIND_SCANOUT | PIPE_BIND_SHARED);
      if (!util_format_is_pure_integer(format))
         retval |= usage & PIPE_BIND_BLENDABLE;
   }

   if ((usage & PIPE_BIND_DEPTH_STENCIL) &&
       (fd_pipe2depth(format) != (enum adreno_rb_depth_format) ~0) &&
       (fd3_pipe2tex(format) != TFMT_NONE)) {
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
static const enum pc_di_primtype primtypes[] = {
   [MESA_PRIM_POINTS]         = DI_PT_POINTLIST,
   [MESA_PRIM_LINES]          = DI_PT_LINELIST,
   [MESA_PRIM_LINE_STRIP]     = DI_PT_LINESTRIP,
   [MESA_PRIM_LINE_LOOP]      = DI_PT_LINELOOP,
   [MESA_PRIM_TRIANGLES]      = DI_PT_TRILIST,
   [MESA_PRIM_TRIANGLE_STRIP] = DI_PT_TRISTRIP,
   [MESA_PRIM_TRIANGLE_FAN]   = DI_PT_TRIFAN,
   [MESA_PRIM_COUNT]            = DI_PT_RECTLIST,  /* internal clear blits */
};
/* clang-format on */

void
fd3_screen_init(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);
   screen->max_rts = A3XX_MAX_RENDER_TARGETS;
   pscreen->context_create = fd3_context_create;
   pscreen->is_format_supported = fd3_screen_is_format_supported;
   fd3_emit_init_screen(pscreen);
   ir3_screen_init(pscreen);

   screen->setup_slices = fd3_setup_slices;
   if (FD_DBG(TTILE))
      screen->tile_mode = fd3_tile_mode;

   screen->primtypes = primtypes;
}
