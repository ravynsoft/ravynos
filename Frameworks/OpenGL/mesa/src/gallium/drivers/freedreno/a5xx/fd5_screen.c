/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
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

#include "fd5_blitter.h"
#include "fd5_context.h"
#include "fd5_emit.h"
#include "fd5_format.h"
#include "fd5_resource.h"
#include "fd5_screen.h"

#include "ir3/ir3_compiler.h"

static bool
valid_sample_count(unsigned sample_count)
{
   switch (sample_count) {
   case 0:
   case 1:
   case 2:
   case 4:
      return true;
   default:
      return false;
   }
}

static bool
fd5_screen_is_format_supported(struct pipe_screen *pscreen,
                               enum pipe_format format,
                               enum pipe_texture_target target,
                               unsigned sample_count,
                               unsigned storage_sample_count, unsigned usage)
{
   unsigned retval = 0;

   if ((target >= PIPE_MAX_TEXTURE_TYPES) ||
       !valid_sample_count(sample_count)) {
      DBG("not supported: format=%s, target=%d, sample_count=%d, usage=%x",
          util_format_name(format), target, sample_count, usage);
      return false;
   }

   if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
      return false;

   if ((usage & PIPE_BIND_VERTEX_BUFFER) &&
       (fd5_pipe2vtx(format) != VFMT5_NONE)) {
      retval |= PIPE_BIND_VERTEX_BUFFER;
   }

   if ((usage & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_SHADER_IMAGE)) &&
       (fd5_pipe2tex(format) != TFMT5_NONE) &&
       (target == PIPE_BUFFER || util_format_get_blocksize(format) != 12)) {
      retval |= usage & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_SHADER_IMAGE);
   }

   if ((usage &
        (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET |
         PIPE_BIND_SCANOUT | PIPE_BIND_SHARED | PIPE_BIND_COMPUTE_RESOURCE)) &&
       (fd5_pipe2color(format) != RB5_NONE) &&
       (fd5_pipe2tex(format) != TFMT5_NONE)) {
      retval |= usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET |
                         PIPE_BIND_SCANOUT | PIPE_BIND_SHARED |
                         PIPE_BIND_COMPUTE_RESOURCE);
   }

   if (usage & PIPE_BIND_SHADER_IMAGE) {
      if (sample_count > 1)
         return false;
   }

   /* For ARB_framebuffer_no_attachments: */
   if ((usage & PIPE_BIND_RENDER_TARGET) && (format == PIPE_FORMAT_NONE)) {
      retval |= usage & PIPE_BIND_RENDER_TARGET;
   }

   if ((usage & PIPE_BIND_DEPTH_STENCIL) &&
       (fd5_pipe2depth(format) != (enum a5xx_depth_format) ~0) &&
       (fd5_pipe2tex(format) != TFMT5_NONE)) {
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
fd5_screen_init(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);
   screen->max_rts = A5XX_MAX_RENDER_TARGETS;
   pscreen->context_create = fd5_context_create;
   pscreen->is_format_supported = fd5_screen_is_format_supported;

   screen->setup_slices = fd5_setup_slices;
   if (FD_DBG(TTILE))
      screen->tile_mode = fd5_tile_mode;

   fd5_emit_init_screen(pscreen);
   ir3_screen_init(pscreen);

   screen->primtypes = primtypes;
}
