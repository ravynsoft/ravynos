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

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/hash_table.h"
#include "util/macros.h"
#include "util/u_debug.h"
#include "util/u_dump.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "util/u_trace_gallium.h"
#include "u_tracepoints.h"

#include "freedreno_context.h"
#include "freedreno_fence.h"
#include "freedreno_gmem.h"
#include "freedreno_query_hw.h"
#include "freedreno_resource.h"
#include "freedreno_tracepoints.h"
#include "freedreno_util.h"

/*
 * GMEM is the small (ie. 256KiB for a200, 512KiB for a220, etc) tile buffer
 * inside the GPU.  All rendering happens to GMEM.  Larger render targets
 * are split into tiles that are small enough for the color (and depth and/or
 * stencil, if enabled) buffers to fit within GMEM.  Before rendering a tile,
 * if there was not a clear invalidating the previous tile contents, we need
 * to restore the previous tiles contents (system mem -> GMEM), and after all
 * the draw calls, before moving to the next tile, we need to save the tile
 * contents (GMEM -> system mem).
 *
 * The code in this file handles dealing with GMEM and tiling.
 *
 * The structure of the ringbuffer ends up being:
 *
 *     +--<---<-- IB ---<---+---<---+---<---<---<--+
 *     |                    |       |              |
 *     v                    ^       ^              ^
 *   ------------------------------------------------------
 *     | clear/draw cmds | Tile0 | Tile1 | .... | TileN |
 *   ------------------------------------------------------
 *                       ^
 *                       |
 *                       address submitted in issueibcmds
 *
 * Where the per-tile section handles scissor setup, mem2gmem restore (if
 * needed), IB to draw cmds earlier in the ringbuffer, and then gmem2mem
 * resolve.
 */

#ifndef BIN_DEBUG
#define BIN_DEBUG 0
#endif

/*
 * GMEM Cache:
 *
 * Caches GMEM state based on a given framebuffer state.  The key is
 * meant to be the minimal set of data that results in a unique gmem
 * configuration, avoiding multiple keys arriving at the same gmem
 * state.  For example, the render target format is not part of the
 * key, only the size per pixel.  And the max_scissor bounds is not
 * part of they key, only the minx/miny (after clamping to tile
 * alignment) and width/height.  This ensures that slightly different
 * max_scissor which would result in the same gmem state, do not
 * become different keys that map to the same state.
 */

struct gmem_key {
   uint16_t minx, miny;
   uint16_t width, height;
   uint8_t gmem_page_align; /* alignment in multiples of 0x1000 to reduce key size */
   uint8_t nr_cbufs;
   uint8_t cbuf_cpp[MAX_RENDER_TARGETS];
   uint8_t zsbuf_cpp[2];
};

static uint32_t
gmem_key_hash(const void *_key)
{
   const struct gmem_key *key = _key;
   return _mesa_hash_data(key, sizeof(*key));
}

static bool
gmem_key_equals(const void *_a, const void *_b)
{
   const struct gmem_key *a = _a;
   const struct gmem_key *b = _b;
   return memcmp(a, b, sizeof(*a)) == 0;
}

static void
dump_gmem_key(const struct gmem_key *key)
{
   printf("{ .minx=%u, .miny=%u, .width=%u, .height=%u", key->minx, key->miny,
          key->width, key->height);
   printf(", .gmem_page_align=%u, .nr_cbufs=%u", key->gmem_page_align,
          key->nr_cbufs);
   printf(", .cbuf_cpp = {");
   for (unsigned i = 0; i < ARRAY_SIZE(key->cbuf_cpp); i++)
      printf("%u,", key->cbuf_cpp[i]);
   printf("}, .zsbuf_cpp = {");
   for (unsigned i = 0; i < ARRAY_SIZE(key->zsbuf_cpp); i++)
      printf("%u,", key->zsbuf_cpp[i]);
   printf("}},\n");
}

static void
dump_gmem_state(const struct fd_gmem_stateobj *gmem)
{
   unsigned total = 0;
   printf("GMEM LAYOUT: bin=%ux%u, nbins=%ux%u\n", gmem->bin_w, gmem->bin_h,
          gmem->nbins_x, gmem->nbins_y);
   for (int i = 0; i < ARRAY_SIZE(gmem->cbuf_base); i++) {
      if (!gmem->cbuf_cpp[i])
         continue;

      unsigned size = gmem->cbuf_cpp[i] * gmem->bin_w * gmem->bin_h;
      printf("  cbuf[%d]: base=0x%06x, size=0x%x, cpp=%u\n", i,
             gmem->cbuf_base[i], size, gmem->cbuf_cpp[i]);

      total = gmem->cbuf_base[i] + size;
   }

   for (int i = 0; i < ARRAY_SIZE(gmem->zsbuf_base); i++) {
      if (!gmem->zsbuf_cpp[i])
         continue;

      unsigned size = gmem->zsbuf_cpp[i] * gmem->bin_w * gmem->bin_h;
      printf("  zsbuf[%d]: base=0x%06x, size=0x%x, cpp=%u\n", i,
             gmem->zsbuf_base[i], size, gmem->zsbuf_cpp[i]);

      total = gmem->zsbuf_base[i] + size;
   }

   printf("total: 0x%06x (of 0x%06x)\n", total, gmem->screen->gmemsize_bytes);
}

static unsigned
div_align(unsigned num, unsigned denom, unsigned al)
{
   return util_align_npot(DIV_ROUND_UP(num, denom), al);
}

static bool
layout_gmem(struct gmem_key *key, uint32_t nbins_x, uint32_t nbins_y,
            struct fd_gmem_stateobj *gmem)
{
   struct fd_screen *screen = gmem->screen;
   uint32_t gmem_align = key->gmem_page_align * 0x1000;
   uint32_t total = 0, i;

   if ((nbins_x == 0) || (nbins_y == 0))
      return false;

   uint32_t bin_w, bin_h;
   bin_w = div_align(key->width, nbins_x, screen->info->tile_align_w);
   bin_h = div_align(key->height, nbins_y, screen->info->tile_align_h);

   if (bin_w > screen->info->tile_max_w)
      return false;

   if (bin_h > screen->info->tile_max_h)
      return false;

   gmem->bin_w = bin_w;
   gmem->bin_h = bin_h;

   /* due to aligning bin_w/h, we could end up with one too
    * many bins in either dimension, so recalculate:
    */
   gmem->nbins_x = DIV_ROUND_UP(key->width, bin_w);
   gmem->nbins_y = DIV_ROUND_UP(key->height, bin_h);

   for (i = 0; i < MAX_RENDER_TARGETS; i++) {
      if (key->cbuf_cpp[i]) {
         gmem->cbuf_base[i] = util_align_npot(total, gmem_align);
         total = gmem->cbuf_base[i] + key->cbuf_cpp[i] * bin_w * bin_h;
      }
   }

   if (key->zsbuf_cpp[0]) {
      gmem->zsbuf_base[0] = util_align_npot(total, gmem_align);
      total = gmem->zsbuf_base[0] + key->zsbuf_cpp[0] * bin_w * bin_h;
   }

   if (key->zsbuf_cpp[1]) {
      gmem->zsbuf_base[1] = util_align_npot(total, gmem_align);
      total = gmem->zsbuf_base[1] + key->zsbuf_cpp[1] * bin_w * bin_h;
   }

   return total <= screen->gmemsize_bytes;
}

static void
calc_nbins(struct gmem_key *key, struct fd_gmem_stateobj *gmem)
{
   struct fd_screen *screen = gmem->screen;
   uint32_t nbins_x = 1, nbins_y = 1;
   uint32_t max_width = screen->info->tile_max_w;
   uint32_t max_height = screen->info->tile_max_h;

   if (FD_DBG(MSGS)) {
      debug_printf("binning input: cbuf cpp:");
      for (unsigned i = 0; i < key->nr_cbufs; i++)
         debug_printf(" %d", key->cbuf_cpp[i]);
      debug_printf(", zsbuf cpp: %d; %dx%d\n", key->zsbuf_cpp[0], key->width,
                   key->height);
   }

   /* first, find a bin size that satisfies the maximum width/
    * height restrictions:
    */
   while (div_align(key->width, nbins_x, screen->info->tile_align_w) >
          max_width) {
      nbins_x++;
   }

   while (div_align(key->height, nbins_y, screen->info->tile_align_h) >
          max_height) {
      nbins_y++;
   }

   /* then find a bin width/height that satisfies the memory
    * constraints:
    */
   while (!layout_gmem(key, nbins_x, nbins_y, gmem)) {
      if (nbins_y > nbins_x) {
         nbins_x++;
      } else {
         nbins_y++;
      }
   }

   /* Lets see if we can tweak the layout a bit and come up with
    * something better:
    */
   if ((((nbins_x - 1) * (nbins_y + 1)) < (nbins_x * nbins_y)) &&
       layout_gmem(key, nbins_x - 1, nbins_y + 1, gmem)) {
      nbins_x--;
      nbins_y++;
   } else if ((((nbins_x + 1) * (nbins_y - 1)) < (nbins_x * nbins_y)) &&
              layout_gmem(key, nbins_x + 1, nbins_y - 1, gmem)) {
      nbins_x++;
      nbins_y--;
   }

   layout_gmem(key, nbins_x, nbins_y, gmem);
}

static struct fd_gmem_stateobj *
gmem_stateobj_init(struct fd_screen *screen, struct gmem_key *key)
{
   struct fd_gmem_stateobj *gmem =
      rzalloc(screen->gmem_cache.ht, struct fd_gmem_stateobj);
   pipe_reference_init(&gmem->reference, 1);
   gmem->screen = screen;
   gmem->key = key;
   list_inithead(&gmem->node);

   const unsigned npipes = screen->info->num_vsc_pipes;
   uint32_t i, j, t, xoff, yoff;
   uint32_t tpp_x, tpp_y;
   int tile_n[npipes];

   calc_nbins(key, gmem);

   DBG("using %d bins of size %dx%d", gmem->nbins_x * gmem->nbins_y,
       gmem->bin_w, gmem->bin_h);

   memcpy(gmem->cbuf_cpp, key->cbuf_cpp, sizeof(key->cbuf_cpp));
   memcpy(gmem->zsbuf_cpp, key->zsbuf_cpp, sizeof(key->zsbuf_cpp));
   gmem->minx = key->minx;
   gmem->miny = key->miny;
   gmem->width = key->width;
   gmem->height = key->height;

   gmem->tile = rzalloc_array(gmem, struct fd_tile, gmem->nbins_x * gmem->nbins_y);

   if (BIN_DEBUG) {
      dump_gmem_state(gmem);
      dump_gmem_key(key);
   }

   /*
    * Assign tiles and pipes:
    *
    * At some point it might be worth playing with different
    * strategies and seeing if that makes much impact on
    * performance.
    */

   /* figure out number of tiles per pipe: */
   if (is_a20x(screen)) {
      /* for a20x we want to minimize the number of "pipes"
       * binning data has 3 bits for x/y (8x8) but the edges are used to
       * cull off-screen vertices with hw binning, so we have 6x6 pipes
       */
      tpp_x = 6;
      tpp_y = 6;
   } else {
      tpp_x = tpp_y = 1;
      while (DIV_ROUND_UP(gmem->nbins_y, tpp_y) > npipes)
         tpp_y += 2;
      while ((DIV_ROUND_UP(gmem->nbins_y, tpp_y) *
              DIV_ROUND_UP(gmem->nbins_x, tpp_x)) > npipes)
         tpp_x += 1;
   }

#ifdef DEBUG
   tpp_x = debug_get_num_option("TPP_X", tpp_x);
   tpp_y = debug_get_num_option("TPP_Y", tpp_x);
#endif

   gmem->maxpw = tpp_x;
   gmem->maxph = tpp_y;

   /* configure pipes: */
   xoff = yoff = 0;
   for (i = 0; i < npipes; i++) {
      struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[i];

      if (xoff >= gmem->nbins_x) {
         xoff = 0;
         yoff += tpp_y;
      }

      if (yoff >= gmem->nbins_y) {
         break;
      }

      pipe->x = xoff;
      pipe->y = yoff;
      pipe->w = MIN2(tpp_x, gmem->nbins_x - xoff);
      pipe->h = MIN2(tpp_y, gmem->nbins_y - yoff);

      xoff += tpp_x;
   }

   /* number of pipes to use for a20x */
   gmem->num_vsc_pipes = MAX2(1, i);

   for (; i < npipes; i++) {
      struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[i];
      pipe->x = pipe->y = pipe->w = pipe->h = 0;
   }

   if (BIN_DEBUG) {
      printf("%dx%d ... tpp=%dx%d\n", gmem->nbins_x, gmem->nbins_y, tpp_x,
             tpp_y);
      for (i = 0; i < ARRAY_SIZE(gmem->vsc_pipe); i++) {
         struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[i];
         printf("pipe[%d]: %ux%u @ %u,%u\n", i, pipe->w, pipe->h, pipe->x,
                pipe->y);
      }
   }

   /* configure tiles: */
   t = 0;
   yoff = key->miny;
   memset(tile_n, 0, sizeof(tile_n));
   for (i = 0; i < gmem->nbins_y; i++) {
      int bw, bh;

      xoff = key->minx;

      /* clip bin height: */
      bh = MIN2(gmem->bin_h, key->miny + key->height - yoff);
      assert(bh > 0);

      for (j = 0; j < gmem->nbins_x; j++) {
         struct fd_tile *tile = &gmem->tile[t];
         uint32_t p;

         /* pipe number: */
         p = ((i / tpp_y) * DIV_ROUND_UP(gmem->nbins_x, tpp_x)) + (j / tpp_x);
         assert(p < gmem->num_vsc_pipes);

         /* clip bin width: */
         bw = MIN2(gmem->bin_w, key->minx + key->width - xoff);
         assert(bw > 0);

         tile->n = !is_a20x(screen) ? tile_n[p]++
                                    : ((i % tpp_y + 1) << 3 | (j % tpp_x + 1));
         tile->p = p;
         tile->bin_w = bw;
         tile->bin_h = bh;
         tile->xoff = xoff;
         tile->yoff = yoff;

         if (BIN_DEBUG) {
            printf("tile[%d]: p=%u, bin=%ux%u+%u+%u\n", t, p, bw, bh, xoff,
                   yoff);
         }

         t++;

         xoff += bw;
      }

      yoff += bh;
   }

   /* Swap the order of alternating rows to form an 'S' pattern, to improve
    * cache access patterns (ie. adjacent bins are likely to access adjacent
    * portions of textures)
    */
   if (!FD_DBG(NOSBIN)) {
      for (i = 0; i < gmem->nbins_y; i+=2) {
         unsigned col0 = gmem->nbins_x * i;
         for (j = 0; j < gmem->nbins_x/2; j++) {
            SWAP(gmem->tile[col0 + j], gmem->tile[col0 + gmem->nbins_x - j - 1]);
         }
      }
   }

   if (BIN_DEBUG) {
      t = 0;
      for (i = 0; i < gmem->nbins_y; i++) {
         for (j = 0; j < gmem->nbins_x; j++) {
            struct fd_tile *tile = &gmem->tile[t++];
            printf("|p:%u n:%u|", tile->p, tile->n);
         }
         printf("\n");
      }
   }

   return gmem;
}

void
__fd_gmem_destroy(struct fd_gmem_stateobj *gmem)
{
   struct fd_gmem_cache *cache = &gmem->screen->gmem_cache;

   fd_screen_assert_locked(gmem->screen);

   _mesa_hash_table_remove_key(cache->ht, gmem->key);
   list_del(&gmem->node);

   ralloc_free(gmem->key);
   ralloc_free(gmem);
}

static struct gmem_key *
gmem_key_init(struct fd_batch *batch, bool assume_zs, bool no_scis_opt)
{
   struct fd_screen *screen = batch->ctx->screen;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   bool has_zs = pfb->zsbuf &&
      !!(batch->gmem_reason & (FD_GMEM_DEPTH_ENABLED | FD_GMEM_STENCIL_ENABLED |
                               FD_GMEM_CLEARS_DEPTH_STENCIL));
   struct gmem_key *key = rzalloc(screen->gmem_cache.ht, struct gmem_key);

   if (has_zs || assume_zs) {
      struct fd_resource *rsc = fd_resource(pfb->zsbuf->texture);
      key->zsbuf_cpp[0] = rsc->layout.cpp * pfb->samples;
      if (rsc->stencil)
         key->zsbuf_cpp[1] = rsc->stencil->layout.cpp * pfb->samples;

      /* If we clear z or s but not both, and we are using z24s8 (ie.
       * !separate_stencil) then we need to restore the other, even if
       * batch_draw_tracking_for_dirty_bits() never saw a draw with
       * depth or stencil enabled.
       *
       * This only applies to the fast-clear path, clears done with
       * u_blitter will show up as a normal draw with depth and/or
       * stencil enabled.
       */
      unsigned zsclear = batch->cleared & (FD_BUFFER_DEPTH | FD_BUFFER_STENCIL);
      if (zsclear) {
         const struct util_format_description *desc =
               util_format_description(pfb->zsbuf->format);
         if (util_format_has_depth(desc) && !(zsclear & FD_BUFFER_DEPTH))
            batch->restore |= FD_BUFFER_DEPTH;
         if (util_format_has_stencil(desc) && !(zsclear & FD_BUFFER_STENCIL))
            batch->restore |= FD_BUFFER_STENCIL;
      }
   } else {
      /* we might have a zsbuf, but it isn't used */
      batch->restore &= ~(FD_BUFFER_DEPTH | FD_BUFFER_STENCIL);
      batch->resolve &= ~(FD_BUFFER_DEPTH | FD_BUFFER_STENCIL);
   }

   key->nr_cbufs = pfb->nr_cbufs;
   for (unsigned i = 0; i < pfb->nr_cbufs; i++) {
      if (pfb->cbufs[i])
         key->cbuf_cpp[i] = util_format_get_blocksize(pfb->cbufs[i]->format);
      else
         key->cbuf_cpp[i] = 4;
      /* if MSAA, color buffers are super-sampled in GMEM: */
      key->cbuf_cpp[i] *= pfb->samples;
   }

   /* NOTE: on a6xx, the max-scissor-rect is handled in fd6_gmem, and
    * we just rely on CP_COND_EXEC to skip bins with no geometry.
    */
   if (no_scis_opt || is_a6xx(screen)) {
      key->minx = 0;
      key->miny = 0;
      key->width = pfb->width;
      key->height = pfb->height;
   } else {
      struct pipe_scissor_state *scissor = &batch->max_scissor;

      if (FD_DBG(NOSCIS)) {
         scissor->minx = 0;
         scissor->miny = 0;
         scissor->maxx = pfb->width - 1;
         scissor->maxy = pfb->height - 1;
      }

      /* round down to multiple of alignment: */
      key->minx = scissor->minx & ~(screen->info->gmem_align_w - 1);
      key->miny = scissor->miny & ~(screen->info->gmem_align_h - 1);
      key->width = scissor->maxx + 1 - key->minx;
      key->height = scissor->maxy + 1 - key->miny;
   }

   if (is_a20x(screen) && batch->cleared) {
      /* under normal circumstances the requirement would be 4K
       * but the fast clear path requires an alignment of 32K
       */
      key->gmem_page_align = 8;
   } else if (is_a6xx(screen)) {
      key->gmem_page_align = screen->info->num_ccu;
   } else {
      // TODO re-check this across gens.. maybe it should only
      // be a single page in some cases:
      key->gmem_page_align = 4;
   }

   return key;
}

static struct fd_gmem_stateobj *
lookup_gmem_state(struct fd_batch *batch, bool assume_zs, bool no_scis_opt)
{
   struct fd_screen *screen = batch->ctx->screen;
   struct fd_gmem_cache *cache = &screen->gmem_cache;
   struct fd_gmem_stateobj *gmem = NULL;

   /* Lock before allocating gmem_key, since that a screen-wide
    * ralloc pool and ralloc itself is not thread-safe.
    */
   fd_screen_lock(screen);

   struct gmem_key *key = gmem_key_init(batch, assume_zs, no_scis_opt);
   uint32_t hash = gmem_key_hash(key);

   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(cache->ht, hash, key);
   if (entry) {
      ralloc_free(key);
      goto found;
   }

   /* limit the # of cached gmem states, discarding the least
    * recently used state if needed:
    */
   if (cache->ht->entries >= 20) {
      struct fd_gmem_stateobj *last =
         list_last_entry(&cache->lru, struct fd_gmem_stateobj, node);
      fd_gmem_reference(&last, NULL);
   }

   entry = _mesa_hash_table_insert_pre_hashed(cache->ht, hash, key,
                                              gmem_stateobj_init(screen, key));

found:
   fd_gmem_reference(&gmem, entry->data);
   /* Move to the head of the LRU: */
   list_delinit(&gmem->node);
   list_add(&gmem->node, &cache->lru);

   fd_screen_unlock(screen);

   return gmem;
}

/*
 * GMEM render pass
 */

static void
render_tiles(struct fd_batch *batch, struct fd_gmem_stateobj *gmem) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   int i;

   simple_mtx_lock(&ctx->gmem_lock);

   ctx->emit_tile_init(batch);

   if (batch->restore)
      ctx->stats.batch_restore++;

   for (i = 0; i < (gmem->nbins_x * gmem->nbins_y); i++) {
      struct fd_tile *tile = &gmem->tile[i];

      trace_start_tile(&batch->trace, batch->gmem, tile->bin_h, tile->yoff, tile->bin_w,
                       tile->xoff);

      ctx->emit_tile_prep(batch, tile);

      if (batch->restore) {
         ctx->emit_tile_mem2gmem(batch, tile);
      }

      ctx->emit_tile_renderprep(batch, tile);

      if (ctx->query_prepare_tile)
         ctx->query_prepare_tile(batch, i, batch->gmem);

      /* emit IB to drawcmds: */
      trace_start_draw_ib(&batch->trace, batch->gmem);
      if (ctx->emit_tile) {
         ctx->emit_tile(batch, tile);
      } else {
         ctx->screen->emit_ib(batch->gmem, batch->draw);
      }
      trace_end_draw_ib(&batch->trace, batch->gmem);
      fd_reset_wfi(batch);

      /* emit gmem2mem to transfer tile back to system memory: */
      ctx->emit_tile_gmem2mem(batch, tile);
   }

   if (ctx->emit_tile_fini)
      ctx->emit_tile_fini(batch);

   simple_mtx_unlock(&ctx->gmem_lock);
}

static void
render_sysmem(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;

   ctx->emit_sysmem_prep(batch);

   if (ctx->query_prepare_tile)
      ctx->query_prepare_tile(batch, 0, batch->gmem);

   if (!batch->nondraw) {
      trace_start_draw_ib(&batch->trace, batch->gmem);
   }
   /* emit IB to drawcmds: */
   if (ctx->emit_sysmem) {
      ctx->emit_sysmem(batch);
   } else {
      ctx->screen->emit_ib(batch->gmem, batch->draw);
   }

   if (!batch->nondraw) {
      trace_end_draw_ib(&batch->trace, batch->gmem);
   }

   fd_reset_wfi(batch);

   if (ctx->emit_sysmem_fini)
      ctx->emit_sysmem_fini(batch);
}

static void
flush_ring(struct fd_batch *batch)
{
   bool use_fence_fd = false;
   if (batch->fence)
      use_fence_fd = batch->fence->use_fence_fd;

   struct fd_fence *fence;

   if (FD_DBG(NOHW)) {
      /* construct a dummy fence: */
      fence = fd_fence_new(batch->ctx->pipe, use_fence_fd);
   } else {
      fence = fd_submit_flush(batch->submit, batch->in_fence_fd, use_fence_fd);
   }

   if (batch->fence) {
      fd_pipe_fence_set_submit_fence(batch->fence, fence);
   } else {
      fd_fence_del(fence);
   }
}

void
fd_gmem_render_tiles(struct fd_batch *batch)
{
   struct fd_context *ctx = batch->ctx;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   bool sysmem = false;

   ctx->submit_count++;

   /* Sometimes we need to flush a batch just to get a fence, with no
    * clears or draws.. in this case promote to nondraw:
    */
   if (!(batch->cleared || batch->num_draws))
      sysmem = true;

   if (!batch->nondraw) {
#if HAVE_PERFETTO
      /* For non-draw batches, we don't really have a good place to
       * match up the api event submit-id to the on-gpu rendering,
       * so skip this for non-draw batches.
       */
      fd_perfetto_submit(ctx);
#endif
      trace_flush_batch(&batch->trace, batch->gmem, batch, batch->cleared,
                        batch->gmem_reason, batch->num_draws);
      trace_framebuffer_state(&batch->trace, batch->gmem, pfb);
   }

   if (ctx->emit_sysmem_prep && !batch->nondraw) {
      if (fd_autotune_use_bypass(&ctx->autotune, batch) && !FD_DBG(GMEM)) {
         sysmem = true;
      }

      /* For ARB_framebuffer_no_attachments: */
      if ((pfb->nr_cbufs == 0) && !pfb->zsbuf) {
         sysmem = true;
      }
   }

   if (FD_DBG(SYSMEM))
      sysmem = true;

   /* Layered rendering always needs bypass. */
   for (unsigned i = 0; i < pfb->nr_cbufs; i++) {
      struct pipe_surface *psurf = pfb->cbufs[i];
      if (!psurf)
         continue;
      if (psurf->u.tex.first_layer < psurf->u.tex.last_layer)
         sysmem = true;
   }
   if (pfb->zsbuf && pfb->zsbuf->u.tex.first_layer < pfb->zsbuf->u.tex.last_layer)
      sysmem = true;

   /* Tessellation doesn't seem to support tiled rendering so fall back to
    * bypass.
    */
   if (batch->tessellation) {
      assert(ctx->emit_sysmem_prep);
      sysmem = true;
   }

   fd_reset_wfi(batch);

   ctx->stats.batch_total++;

   if (batch->nondraw) {
      DBG("%p: rendering non-draw", batch);
      if (!fd_ringbuffer_empty(batch->draw))
         render_sysmem(batch);
      ctx->stats.batch_nondraw++;
   } else if (sysmem) {
      trace_render_sysmem(&batch->trace, batch->gmem);
      trace_start_render_pass(&batch->trace, batch->gmem,
         ctx->submit_count, pipe_surface_format(pfb->cbufs[0]),
         pipe_surface_format(pfb->zsbuf), pfb->width, pfb->height,
         pfb->nr_cbufs, pfb->samples, 0, 0, 0);
      if (ctx->query_prepare)
         ctx->query_prepare(batch, 1);
      render_sysmem(batch);
      trace_end_render_pass(&batch->trace, batch->gmem);
      ctx->stats.batch_sysmem++;
   } else {
      struct fd_gmem_stateobj *gmem = lookup_gmem_state(batch, false, false);
      batch->gmem_state = gmem;
      trace_render_gmem(&batch->trace, batch->gmem, gmem->nbins_x, gmem->nbins_y,
                        gmem->bin_w, gmem->bin_h);
      trace_start_render_pass(&batch->trace, batch->gmem,
         ctx->submit_count, pipe_surface_format(pfb->cbufs[0]),
         pipe_surface_format(pfb->zsbuf), pfb->width, pfb->height,
         pfb->nr_cbufs, pfb->samples, gmem->nbins_x * gmem->nbins_y,
         gmem->bin_w, gmem->bin_h);
      if (ctx->query_prepare)
         ctx->query_prepare(batch, gmem->nbins_x * gmem->nbins_y);
      render_tiles(batch, gmem);
      trace_end_render_pass(&batch->trace, batch->gmem);
      batch->gmem_state = NULL;

      fd_screen_lock(ctx->screen);
      fd_gmem_reference(&gmem, NULL);
      fd_screen_unlock(ctx->screen);

      ctx->stats.batch_gmem++;
   }

   flush_ring(batch);

   u_trace_flush(&batch->trace, NULL, false);
}

/* Determine a worst-case estimate (ie. assuming we don't eliminate an
 * unused depth/stencil) number of bins per vsc pipe.
 */
unsigned
fd_gmem_estimate_bins_per_pipe(struct fd_batch *batch)
{
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd_screen *screen = batch->ctx->screen;
   struct fd_gmem_stateobj *gmem = lookup_gmem_state(batch, !!pfb->zsbuf, true);
   unsigned nbins = gmem->maxpw * gmem->maxph;

   fd_screen_lock(screen);
   fd_gmem_reference(&gmem, NULL);
   fd_screen_unlock(screen);

   return nbins;
}

/* When deciding whether a tile needs mem2gmem, we need to take into
 * account the scissor rect(s) that were cleared.  To simplify we only
 * consider the last scissor rect for each buffer, since the common
 * case would be a single clear.
 */
bool
fd_gmem_needs_restore(struct fd_batch *batch, const struct fd_tile *tile,
                      uint32_t buffers)
{
   if (!(batch->restore & buffers))
      return false;

   return true;
}

void
fd_gmem_screen_init(struct pipe_screen *pscreen)
{
   struct fd_gmem_cache *cache = &fd_screen(pscreen)->gmem_cache;

   cache->ht = _mesa_hash_table_create(NULL, gmem_key_hash, gmem_key_equals);
   list_inithead(&cache->lru);
}

void
fd_gmem_screen_fini(struct pipe_screen *pscreen)
{
   struct fd_gmem_cache *cache = &fd_screen(pscreen)->gmem_cache;

   _mesa_hash_table_destroy(cache->ht, NULL);
}
