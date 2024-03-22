/*
 * Copyright © 2020 Google, Inc.
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
 */

#define FD_BO_NO_HARDPIN 1

#include "pipe/p_state.h"

#include "freedreno_batch.h"
#include "freedreno_gmem.h"

#include "fd6_vsc.h"

/*
 * Helper util to update expected vsc draw and primitive stream sizes, see
 * https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/Visibility-Stream-Format
 */

enum bits_per {
   byte = 8,
   dword = 4 * byte,
};

/**
 * Determine # of bits required to store a given number, see
 * https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/Visibility-Stream-Format#numbers
 */
static unsigned
number_size_bits(unsigned nr)
{
   unsigned n = util_last_bit(nr);
   assert(n); /* encoding 0 is not possible */
   return n + (n - 1);
}

/**
 * Determine # of bits requred to store a given bitfield, see
 * https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/Visibility-Stream-Format#bitfields
 */
static unsigned
bitfield_size_bits(unsigned n)
{
   return n + 1; /* worst case is always 1 + nr of bits */
}

static unsigned
prim_count(const struct pipe_draw_info *info,
           const struct pipe_draw_start_count_bias *draw)
{
   /* MESA_PRIM_COUNT used internally for RECTLIST blits on 3d pipe: */
   unsigned vtx_per_prim =
      (info->mode == MESA_PRIM_COUNT) ? 2 : mesa_vertices_per_prim(info->mode);
   return MAX2(1, (draw->count * info->instance_count) / vtx_per_prim);
}

/**
 * The primitive stream uses a run-length encoding, where each packet contains a
 * bitfield of bins covered and then the number of primitives which have the
 * same bitfield. Each packet consists of the following, in order:
 *
 *  - The (compressed) bitfield of bins covered
 *  - The number of primitives with this bitset
 *  - Checksum
 *
 * The worst case would be that each primitive has a different bitmask.  In
 * practice, assuming ever other primitive has a different bitmask still gets us
 * conservatively large primitive stream sizes.  (Ie. 10x what is needed, vs.
 * 20x)
 *
 * https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/Visibility-Stream-Format#primitive-streams
 */
static unsigned
primitive_stream_size_bits(const struct pipe_draw_info *info,
                           const struct pipe_draw_start_count_bias *draw,
                           unsigned num_bins)
{
   unsigned num_prims = prim_count(info, draw);
   unsigned nbits =
      (bitfield_size_bits(num_bins) /* bitfield of bins covered */
       + number_size_bits(1)        /* number of primitives with this bitset */
       + 1                          /* checksum */
       ) *
      DIV_ROUND_UP(num_prims, 2);
   return align(nbits, dword);
}

/**
 * Each draw stream packet contains the following:
 *
 *  - Bin bitfield
 *  - Last instance bit
 *  - If bitfield is empty, the number of draws it is empty for, otherwise
 *    the size of the corresponding primitive stream in DWORD's.
 *  - Checksum
 *
 * https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/Visibility-Stream-Format#draw-streams
 */
static unsigned
draw_stream_size_bits(const struct pipe_draw_info *info, unsigned num_bins,
                      unsigned prim_strm_bits)
{
   unsigned ndwords = prim_strm_bits / dword;
   return (bitfield_size_bits(num_bins) /* bitfield of bins */
           + 1                          /* last-instance-bit */
           + number_size_bits(ndwords)  /* size of corresponding prim strm */
           + 1                          /* checksum */
           ) *
          MAX2(1, info->instance_count);
}

void
fd6_vsc_update_sizes(struct fd_batch *batch, const struct pipe_draw_info *info,
                     const struct pipe_draw_start_count_bias *draw)
{
   if (!batch->num_bins_per_pipe) {
      batch->num_bins_per_pipe = fd_gmem_estimate_bins_per_pipe(batch);

      /* This is a convenient spot to add the size of the final draw-
       * stream packet:
       *
       * If there are N bins, the final packet, after all the draws are
       * done, consists of a 1 followed by N + 17 0's, plus a final 1.
       * This uses the otherwise-unused pattern of a non-empty bitfield
       * (initial 1) that is nontheless empty (has all 0's)
       */
      unsigned final_pkt_sz = 1 + batch->num_bins_per_pipe + 17 + 1;
      batch->prim_strm_bits = align(final_pkt_sz, dword);
   }

   unsigned prim_strm_bits =
      primitive_stream_size_bits(info, draw, batch->num_bins_per_pipe);
   unsigned draw_strm_bits =
      draw_stream_size_bits(info, batch->num_bins_per_pipe, prim_strm_bits);

#if 0
   mesa_logd("vsc: prim_strm_bits=%d, draw_strm_bits=%d, nb=%u, ic=%u, c=%u, pc=%u (%s)",
             prim_strm_bits, draw_strm_bits, batch->num_bins_per_pipe,
             info->instance_count, info->count,
             (info->count * info->instance_count) /
             mesa_vertices_per_prim(info->mode),
             u_prim_name(info->mode));
#endif

   batch->prim_strm_bits += prim_strm_bits;
   batch->draw_strm_bits += draw_strm_bits;
}
