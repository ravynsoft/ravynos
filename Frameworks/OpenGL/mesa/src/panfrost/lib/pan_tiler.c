/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "util/macros.h"
#include "util/u_math.h"
#include "pan_device.h"
#include "pan_encoder.h"

/* Mali GPUs are tiled-mode renderers, rather than immediate-mode.
 * Conceptually, the screen is divided into 16x16 tiles. Vertex shaders run.
 * Then, a fixed-function hardware block (the tiler) consumes the gl_Position
 * results. For each triangle specified, it marks each containing tile as
 * containing that triangle. This set of "triangles per tile" form the "polygon
 * list". Finally, the rasterization unit consumes the polygon list to invoke
 * the fragment shader.
 *
 * In practice, it's a bit more complicated than this. On Midgard chips with an
 * "advanced tiling unit" (all except T720/T820/T830), 16x16 is the logical
 * tile size, but Midgard features "hierarchical tiling", where power-of-two
 * multiples of the base tile size can be used: hierarchy level 0 (16x16),
 * level 1 (32x32), level 2 (64x64), per public information about Midgard's
 * tiling. In fact, tiling goes up to 4096x4096 (!), although in practice
 * 128x128 is the largest usually used (though higher modes are enabled).  The
 * idea behind hierarchical tiling is to use low tiling levels for small
 * triangles and high levels for large triangles, to minimize memory bandwidth
 * and repeated fragment shader invocations (the former issue inherent to
 * immediate-mode rendering and the latter common in traditional tilers).
 *
 * The tiler itself works by reading varyings in and writing a polygon list
 * out. Unfortunately (for us), both of these buffers are managed in main
 * memory; although they ideally will be cached, it is the drivers'
 * responsibility to allocate these buffers. Varying buffer allocation is
 * handled elsewhere, as it is not tiler specific; the real issue is allocating
 * the polygon list.
 *
 * This is hard, because from the driver's perspective, we have no information
 * about what geometry will actually look like on screen; that information is
 * only gained from running the vertex shader. (Theoretically, we could run the
 * vertex shaders in software as a prepass, or in hardware with transform
 * feedback as a prepass, but either idea is ludicrous on so many levels).
 *
 * Instead, Mali uses a bit of a hybrid approach, splitting the polygon list
 * into three distinct pieces. First, the driver statically determines which
 * tile hierarchy levels to use (more on that later). At this point, we know the
 * framebuffer dimensions and all the possible tilings of the framebuffer, so
 * we know exactly how many tiles exist across all hierarchy levels. The first
 * piece of the polygon list is the header, which is exactly 8 bytes per tile,
 * plus padding and a small 64-byte prologue. (If that doesn't remind you of
 * AFBC, it should. See pan_afbc.c for some fun parallels). The next part is
 * the polygon list body, which seems to contain 512 bytes per tile, again
 * across every level of the hierarchy. These two parts form the polygon list
 * buffer. This buffer has a statically determinable size, approximately equal
 * to the # of tiles across all hierarchy levels * (8 bytes + 512 bytes), plus
 * alignment / minimum restrictions / etc.
 *
 * The third piece is the easy one (for us): the tiler heap. In essence, the
 * tiler heap is a gigantic slab that's as big as could possibly be necessary
 * in the worst case imaginable. Just... a gigantic allocation that we give a
 * start and end pointer to. What's the catch? The tiler heap is lazily
 * allocated; that is, a huge amount of memory is _reserved_, but only a tiny
 * bit is actually allocated upfront. The GPU just keeps using the
 * unallocated-but-reserved portions as it goes along, generating page faults
 * if it goes beyond the allocation, and then the kernel is instructed to
 * expand the allocation on page fault (known in the vendor kernel as growable
 * memory). This is quite a bit of bookkeeping of its own, but that task is
 * pushed to kernel space and we can mostly ignore it here, just remembering to
 * set the GROWABLE flag so the kernel actually uses this path rather than
 * allocating a gigantic amount up front and burning a hole in RAM.
 *
 * As far as determining which hierarchy levels to use, the simple answer is
 * that right now, we don't. In the tiler configuration fields (consistent from
 * the earliest Midgard's SFBD through the latest Bifrost traces we have),
 * there is a hierarchy_mask field, controlling which levels (tile sizes) are
 * enabled. Ideally, the hierarchical tiling dream -- mapping big polygons to
 * big tiles and small polygons to small tiles -- would be realized here as
 * well. As long as there are polygons at all needing tiling, we always have to
 * have big tiles available, in case there are big polygons. But we don't
 * necessarily need small tiles available. Ideally, when there are small
 * polygons, small tiles are enabled (to avoid waste from putting small
 * triangles in the big tiles); when there are not, small tiles are disabled to
 * avoid enabling more levels than necessary, which potentially costs in memory
 * bandwidth / power / tiler performance.
 *
 * Of course, the driver has to figure this out statically. When tile
 * hiearchies are actually established, this occurs by the tiler in
 * fixed-function hardware, after the vertex shaders have run and there is
 * sufficient information to figure out the size of triangles. The driver has
 * no such luxury, again barring insane hacks like additionally running the
 * vertex shaders in software or in hardware via transform feedback. Thus, for
 * the driver, we need a heuristic approach.
 *
 * There are lots of heuristics to guess triangle size statically you could
 * imagine, but one approach shines as particularly simple-stupid: assume all
 * on-screen triangles are equal size and spread equidistantly throughout the
 * screen. Let's be clear, this is NOT A VALID ASSUMPTION. But if we roll with
 * it, then we see:
 *
 *      Triangle Area   = (Screen Area / # of triangles)
 *                      = (Width * Height) / (# of triangles)
 *
 * Or if you prefer, we can also make a third CRAZY assumption that we only draw
 * right triangles with edges parallel/perpendicular to the sides of the screen
 * with no overdraw, forming a triangle grid across the screen:
 *
 * |--w--|
 *  _____   |
 * | /| /|  |
 * |/_|/_|  h
 * | /| /|  |
 * |/_|/_|  |
 *
 * Then you can use some middle school geometry and algebra to work out the
 * triangle dimensions. I started working on this, but realised I didn't need
 * to to make my point, but couldn't bare to erase that ASCII art. Anyway.
 *
 * POINT IS, by considering the ratio of screen area and triangle count, we can
 * estimate the triangle size. For a small size, use small bins; for a large
 * size, use large bins. Intuitively, this metric makes sense: when there are
 * few triangles on a large screen, you're probably compositing a UI and
 * therefore the triangles are large; when there are a lot of triangles on a
 * small screen, you're probably rendering a 3D mesh and therefore the
 * triangles are tiny. (Or better said -- there will be tiny triangles, even if
 * there are also large triangles. There have to be unless you expect crazy
 * overdraw. Generally, it's better to allow more small bin sizes than
 * necessary than not allow enough.)
 *
 * From this heuristic (or whatever), we determine the minimum allowable tile
 * size, and we use that to decide the hierarchy masking, selecting from the
 * minimum "ideal" tile size to the maximum tile size (2048x2048 in practice).
 *
 * Once we have that mask and the framebuffer dimensions, we can compute the
 * size of the statically-sized polygon list structures, allocate them, and go!
 *
 * -----
 *
 * On T720, T820, and T830, there is no support for hierarchical tiling.
 * Instead, the hardware allows the driver to select the tile size dynamically
 * on a per-framebuffer basis, including allowing rectangular/non-square tiles.
 * Rules for tile size selection are as follows:
 *
 *  - Dimensions must be powers-of-two.
 *  - The smallest tile is 16x16.
 *  - The tile width/height is at most the framebuffer w/h (clamp up to 16 pix)
 *  - There must be no more than 64 tiles in either dimension.
 *
 * Within these constraints, the driver is free to pick a tile size according
 * to some heuristic, similar to units with an advanced tiling unit.
 *
 * To pick a size without any heuristics, we may satisfy the constraints by
 * defaulting to 16x16 (a power-of-two). This fits the minimum. For the size
 * constraint, consider:
 *
 *      # of tiles < 64
 *      ceil (fb / tile) < 64
 *      (fb / tile) <= (64 - 1)
 *      tile <= fb / (64 - 1) <= next_power_of_two(fb / (64 - 1))
 *
 * Hence we clamp up to align_pot(fb / (64 - 1)).

 * Extending to use a selection heuristic left for future work.
 *
 * Once the tile size (w, h) is chosen, we compute the hierarchy "mask":
 *
 *      hierarchy_mask = (log2(h / 16) << 6) | log2(w / 16)
 *
 * Of course with no hierarchical tiling, this is not a mask; it's just a field
 * specifying the tile size. But I digress.
 *
 * We also compute the polgon list sizes (with framebuffer size W, H) as:
 *
 *      full_size = 0x200 + 0x200 * ceil(W / w) * ceil(H / h)
 *      offset = 8 * ceil(W / w) * ceil(H / h)
 *
 * It further appears necessary to round down offset to the nearest 0x200.
 * Possibly we would also round down full_size to the nearest 0x200 but
 * full_size/0x200 = (1 + ceil(W / w) * ceil(H / h)) is an integer so there's
 * nothing to do.
 */

/* Hierarchical tiling spans from 16x16 to 4096x4096 tiles */

#define MIN_TILE_SIZE 16
#define MAX_TILE_SIZE 4096

/* Constants as shifts for easier power-of-two iteration */

#define MIN_TILE_SHIFT util_logbase2(MIN_TILE_SIZE)
#define MAX_TILE_SHIFT util_logbase2(MAX_TILE_SIZE)

/* The hierarchy has a 64-byte prologue */
#define PROLOGUE_SIZE 0x40

/* For each tile (across all hierarchy levels), there is 8 bytes of header */
#define HEADER_BYTES_PER_TILE 0x8

/* Likewise, each tile per level has 512 bytes of body */
#define FULL_BYTES_PER_TILE 0x200

static unsigned
panfrost_hierarchy_size(unsigned width, unsigned height, unsigned mask,
                        unsigned bytes_per_tile)
{
   unsigned size = PROLOGUE_SIZE;

   /* Iterate hierarchy levels */
   u_foreach_bit(level, mask) {
      assert(level <= (MAX_TILE_SHIFT - MIN_TILE_SHIFT) &&
             "invalid hierarchy mask");

      /* Levels are power-of-two sizes */
      unsigned tile_size = MIN_TILE_SIZE << level;

      size += DIV_ROUND_UP(width, tile_size) * DIV_ROUND_UP(height, tile_size) *
              bytes_per_tile;
   }

   /* This size will be used as an offset, so ensure it's aligned */
   return ALIGN_POT(size, 0x200);
}

/* Implement the formula:
 *
 *      0x200 + bytes_per_tile * ceil(W / w) * ceil(H / h)
 *
 * rounding down the answer to the nearest 0x200. This is used to compute both
 * header and body sizes for GPUs without hierarchical tiling. Essentially,
 * computing a single hierarchy level, since there isn't any hierarchy!
 */

static unsigned
panfrost_flat_size(unsigned width, unsigned height, unsigned dim,
                   unsigned bytes_per_tile)
{
   /* First, extract the tile dimensions */
   unsigned tw = (1 << (dim & 0b111)) * 8;
   unsigned th = (1 << ((dim & (0b111 << 6)) >> 6)) * 8;

   /* Calculate the raw size */
   unsigned raw =
      DIV_ROUND_UP(width, tw) * DIV_ROUND_UP(height, th) * bytes_per_tile;

   /* Round down and add offset */
   return 0x200 + ((raw / 0x200) * 0x200);
}

/* Given a hierarchy mask and a framebuffer size, compute the header size */

unsigned
panfrost_tiler_header_size(unsigned width, unsigned height, unsigned mask,
                           bool hierarchy)
{
   if (hierarchy)
      return panfrost_hierarchy_size(width, height, mask,
                                     HEADER_BYTES_PER_TILE);
   else
      return panfrost_flat_size(width, height, mask, HEADER_BYTES_PER_TILE);
}

/* The combined header/body is sized similarly (but it is significantly
 * larger), except that it can be empty when the tiler disabled, rather than
 * getting clamped to a minimum size.
 */

unsigned
panfrost_tiler_full_size(unsigned width, unsigned height, unsigned mask,
                         bool hierarchy)
{
   if (hierarchy)
      return panfrost_hierarchy_size(width, height, mask, FULL_BYTES_PER_TILE);
   else
      return panfrost_flat_size(width, height, mask, FULL_BYTES_PER_TILE);
}

/* On GPUs without hierarchical tiling, we choose a tile size directly and
 * stuff it into the field otherwise known as hierarchy mask (not a mask). */

static unsigned
panfrost_choose_tile_size(unsigned width, unsigned height,
                          unsigned vertex_count)
{
   /* Figure out the ideal tile size. Eventually a heuristic should be
    * used for this */

   unsigned best_w = 16;
   unsigned best_h = 16;

   /* Clamp so there are less than 64 tiles in each direction */

   best_w = MAX2(best_w, util_next_power_of_two(width / 63));
   best_h = MAX2(best_h, util_next_power_of_two(height / 63));

   /* We have our ideal tile size, so encode */

   unsigned exp_w = util_logbase2(best_w / 16);
   unsigned exp_h = util_logbase2(best_h / 16);

   return exp_w | (exp_h << 6);
}

unsigned
panfrost_choose_hierarchy_mask(unsigned width, unsigned height,
                               unsigned vertex_count, bool hierarchy)
{
   /* If there is no geometry, we don't bother enabling anything */

   if (!vertex_count)
      return 0x00;

   if (!hierarchy)
      return panfrost_choose_tile_size(width, height, vertex_count);

   /* Heuristic: choose the largest minimum bin size such that there are an
    * average of k vertices per bin at the lowest level. This is modeled as:
    *
    *    k = vertex_count / ((fb width / bin width) * (fb height / bin height))
    *
    * Bins are square, so solving for bin size = bin width = bin height:
    *
    *    bin size = sqrt(((k) (fb width) (fb height) / vertex count))
    *
    * k = 4 represents each bin as a QUAD. If the screen is completely tiled
    * into nonoverlapping uniform power-of-two squares, then this heuristic sets
    * the bin size to the quad size, which seems like an ok choice.
    */
   unsigned k = 4;
   unsigned log2_min_bin_size =
      util_logbase2_ceil((k * width * height) / vertex_count) / 2;

   /* Do not use bins larger than the framebuffer. They will be empty. */
   unsigned log2_max_bin_size = util_logbase2_ceil(MAX2(width, height));

   /* For small framebuffers, use one big tile */
   log2_min_bin_size = MIN2(log2_min_bin_size, log2_max_bin_size);

   /* Clamp to valid bin sizes */
   log2_min_bin_size = CLAMP(log2_min_bin_size, MIN_TILE_SHIFT, MAX_TILE_SHIFT);
   log2_max_bin_size = CLAMP(log2_max_bin_size, MIN_TILE_SHIFT, MAX_TILE_SHIFT);

   /* Bin indices are numbered from 0 started with MIN_TILE_SIZE */
   unsigned min_bin_index = log2_min_bin_size - MIN_TILE_SHIFT;
   unsigned max_bin_index = log2_max_bin_size - MIN_TILE_SHIFT;

   /* Enable up to 8 bins starting from the heuristic selected minimum. 8
    * is the implementation specific maximum in supported Midgard devices.
    */
   unsigned mask =
      (BITFIELD_MASK(8) << min_bin_index) & BITFIELD_MASK(max_bin_index + 1);

   assert(mask != 0 && "too few levels");
   assert(util_bitcount(mask) <= 8 && "too many levels");

   return mask;
}
