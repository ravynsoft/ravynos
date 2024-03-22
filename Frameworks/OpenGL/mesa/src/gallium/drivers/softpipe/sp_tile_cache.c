/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 * Render target tile caching.
 *
 * Author:
 *    Brian Paul
 */

#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_tile.h"
#include "sp_tile_cache.h"

static struct softpipe_cached_tile *
sp_alloc_tile(struct softpipe_tile_cache *tc);


/**
 * Return the position in the cache for the tile that contains win pos (x,y).
 * We currently use a direct mapped cache so this is like a hack key.
 * At some point we should investigate something more sophisticated, like
 * a LRU replacement policy.
 */
#define CACHE_POS(x, y, l)                        \
   (((x) + (y) * 5 + (l) * 10) % NUM_ENTRIES)


static inline int addr_to_clear_pos(union tile_address addr)
{
   int pos;
   pos = addr.bits.layer * (MAX_WIDTH / TILE_SIZE) * (MAX_HEIGHT / TILE_SIZE);
   pos += addr.bits.y * (MAX_WIDTH / TILE_SIZE);
   pos += addr.bits.x;
   return pos;
}
/**
 * Is the tile at (x,y) in cleared state?
 */
static inline uint
is_clear_flag_set(const uint *bitvec, union tile_address addr, unsigned max)
{
   int pos, bit;
   pos = addr_to_clear_pos(addr);
   assert(pos / 32 < max);
   bit = bitvec[pos / 32] & (1 << (pos & 31));
   return bit;
}
   

/**
 * Mark the tile at (x,y) as not cleared.
 */
static inline void
clear_clear_flag(uint *bitvec, union tile_address addr, unsigned max)
{
   int pos;
   pos = addr_to_clear_pos(addr);
   assert(pos / 32 < max);
   bitvec[pos / 32] &= ~(1 << (pos & 31));
}
   

struct softpipe_tile_cache *
sp_create_tile_cache( struct pipe_context *pipe )
{
   struct softpipe_tile_cache *tc;
   uint pos;

   /* sanity checking: max sure MAX_WIDTH/HEIGHT >= largest texture image */
   assert(MAX_WIDTH >= pipe->screen->get_param(pipe->screen,
                                               PIPE_CAP_MAX_TEXTURE_2D_SIZE));

   STATIC_ASSERT(sizeof(union tile_address) == 4);

   STATIC_ASSERT((TILE_SIZE << TILE_ADDR_BITS) >= MAX_WIDTH);

   tc = CALLOC_STRUCT( softpipe_tile_cache );
   if (tc) {
      tc->pipe = pipe;
      for (pos = 0; pos < ARRAY_SIZE(tc->tile_addrs); pos++) {
         tc->tile_addrs[pos].bits.invalid = 1;
      }
      tc->last_tile_addr.bits.invalid = 1;

      /* this allocation allows us to guarantee that allocation
       * failures are never fatal later
       */
      tc->tile = MALLOC_STRUCT( softpipe_cached_tile );
      if (!tc->tile)
      {
         FREE(tc);
         return NULL;
      }

      /* XXX this code prevents valgrind warnings about use of uninitialized
       * memory in programs that don't clear the surface before rendering.
       * However, it breaks clearing in other situations (such as in
       * progs/tests/drawbuffers, see bug 24402).
       */
#if 0
      /* set flags to indicate all the tiles are cleared */
      memset(tc->clear_flags, 255, sizeof(tc->clear_flags));
#endif
   }
   return tc;
}


void
sp_destroy_tile_cache(struct softpipe_tile_cache *tc)
{
   if (tc) {
      uint pos;

      for (pos = 0; pos < ARRAY_SIZE(tc->entries); pos++) {
         /*assert(tc->entries[pos].x < 0);*/
         FREE( tc->entries[pos] );
      }
      FREE( tc->tile );

      if (tc->num_maps) {
         int i;
         for (i = 0; i < tc->num_maps; i++)
            if (tc->transfer[i]) {
               tc->pipe->texture_unmap(tc->pipe, tc->transfer[i]);
            }
         FREE(tc->transfer);
         FREE(tc->transfer_map);
         FREE(tc->clear_flags);
      }

      FREE( tc );
   }
}


/**
 * Specify the surface to cache.
 */
void
sp_tile_cache_set_surface(struct softpipe_tile_cache *tc,
                          struct pipe_surface *ps)
{
   struct pipe_context *pipe = tc->pipe;
   int i;

   if (tc->num_maps) {
      if (ps == tc->surface)
         return;

      for (i = 0; i < tc->num_maps; i++) {
         pipe->texture_unmap(pipe, tc->transfer[i]);
         tc->transfer[i] = NULL;
         tc->transfer_map[i] = NULL;
      }
      FREE(tc->transfer);
      FREE(tc->transfer_map);
      tc->num_maps = 0;

      FREE(tc->clear_flags);
      tc->clear_flags_size = 0;
   }

   tc->surface = ps;

   if (ps) {
      tc->num_maps = ps->u.tex.last_layer - ps->u.tex.first_layer + 1;
      tc->transfer = CALLOC(tc->num_maps, sizeof(struct pipe_transfer *));
      tc->transfer_map = CALLOC(tc->num_maps, sizeof(void *));

      tc->clear_flags_size = (MAX_WIDTH / TILE_SIZE) * (MAX_HEIGHT / TILE_SIZE) * tc->num_maps / 32 * sizeof(uint);
      tc->clear_flags = CALLOC(1, tc->clear_flags_size);

      if (ps->texture->target != PIPE_BUFFER) {
         for (i = 0; i < tc->num_maps; i++) {
            tc->transfer_map[i] = pipe_texture_map(pipe, ps->texture,
                                                    ps->u.tex.level, ps->u.tex.first_layer + i,
                                                    PIPE_MAP_READ_WRITE |
                                                    PIPE_MAP_UNSYNCHRONIZED,
                                                    0, 0, ps->width, ps->height,
                                                    &tc->transfer[i]);
         }
      }
      else {
         /* can't render to buffers */
         assert(0);
      }

      tc->depth_stencil = util_format_is_depth_or_stencil(ps->format);
   }
}


/**
 * Return the transfer being cached.
 */
struct pipe_surface *
sp_tile_cache_get_surface(struct softpipe_tile_cache *tc)
{
   return tc->surface;
}


/**
 * Set pixels in a tile to the given clear color/value, float.
 */
static void
clear_tile_rgba(struct softpipe_cached_tile *tile,
                enum pipe_format format,
                const union pipe_color_union *clear_value)
{
   if (clear_value->f[0] == 0.0 &&
       clear_value->f[1] == 0.0 &&
       clear_value->f[2] == 0.0 &&
       clear_value->f[3] == 0.0) {
      memset(tile->data.color, 0, sizeof(tile->data.color));
   }
   else {
      uint i, j;

      if (util_format_is_pure_uint(format)) {
         for (i = 0; i < TILE_SIZE; i++) {
            for (j = 0; j < TILE_SIZE; j++) {
               tile->data.colorui128[i][j][0] = clear_value->ui[0];
               tile->data.colorui128[i][j][1] = clear_value->ui[1];
               tile->data.colorui128[i][j][2] = clear_value->ui[2];
               tile->data.colorui128[i][j][3] = clear_value->ui[3];
            }
         }
      } else if (util_format_is_pure_sint(format)) {
         for (i = 0; i < TILE_SIZE; i++) {
            for (j = 0; j < TILE_SIZE; j++) {
               tile->data.colori128[i][j][0] = clear_value->i[0];
               tile->data.colori128[i][j][1] = clear_value->i[1];
               tile->data.colori128[i][j][2] = clear_value->i[2];
               tile->data.colori128[i][j][3] = clear_value->i[3];
            }
         }
      } else {
         for (i = 0; i < TILE_SIZE; i++) {
            for (j = 0; j < TILE_SIZE; j++) {
               tile->data.color[i][j][0] = clear_value->f[0];
               tile->data.color[i][j][1] = clear_value->f[1];
               tile->data.color[i][j][2] = clear_value->f[2];
               tile->data.color[i][j][3] = clear_value->f[3];
            }
         }
      }
   }
}


/**
 * Set a tile to a solid value/color.
 */
static void
clear_tile(struct softpipe_cached_tile *tile,
           enum pipe_format format,
           uint64_t clear_value)
{
   uint i, j;

   switch (util_format_get_blocksize(format)) {
   case 1:
      memset(tile->data.any, (int) clear_value, TILE_SIZE * TILE_SIZE);
      break;
   case 2:
      if (clear_value == 0) {
         memset(tile->data.any, 0, 2 * TILE_SIZE * TILE_SIZE);
      }
      else {
         for (i = 0; i < TILE_SIZE; i++) {
            for (j = 0; j < TILE_SIZE; j++) {
               tile->data.depth16[i][j] = (uint16_t) clear_value;
            }
         }
      }
      break;
   case 4:
      if (clear_value == 0) {
         memset(tile->data.any, 0, 4 * TILE_SIZE * TILE_SIZE);
      }
      else {
         for (i = 0; i < TILE_SIZE; i++) {
            for (j = 0; j < TILE_SIZE; j++) {
               tile->data.depth32[i][j] = (uint) clear_value;
            }
         }
      }
      break;
   case 8:
      if (clear_value == 0) {
         memset(tile->data.any, 0, 8 * TILE_SIZE * TILE_SIZE);
      }
      else {
         for (i = 0; i < TILE_SIZE; i++) {
            for (j = 0; j < TILE_SIZE; j++) {
               tile->data.depth64[i][j] = clear_value;
            }
         }
      }
      break;
   default:
      assert(0);
   }
}


/**
 * Actually clear the tiles which were flagged as being in a clear state.
 */
static void
sp_tile_cache_flush_clear(struct softpipe_tile_cache *tc, int layer)
{
   struct pipe_transfer *pt = tc->transfer[layer];
   const uint w = tc->transfer[layer]->box.width;
   const uint h = tc->transfer[layer]->box.height;
   uint x, y;
   UNUSED uint numCleared = 0;

   assert(pt->resource);

   /* clear the scratch tile to the clear value */
   if (tc->depth_stencil) {
      clear_tile(tc->tile, pt->resource->format, tc->clear_val);
   } else {
      clear_tile_rgba(tc->tile, pt->resource->format, &tc->clear_color);
   }

   /* push the tile to all positions marked as clear */
   for (y = 0; y < h; y += TILE_SIZE) {
      for (x = 0; x < w; x += TILE_SIZE) {
         union tile_address addr = tile_address(x, y, layer);

         if (is_clear_flag_set(tc->clear_flags, addr, tc->clear_flags_size)) {
            /* write the scratch tile to the surface */
            if (tc->depth_stencil) {
               pipe_put_tile_raw(pt, tc->transfer_map[layer],
                                 x, y, TILE_SIZE, TILE_SIZE,
                                 tc->tile->data.any, 0/*STRIDE*/);
            }
            else {
               pipe_put_tile_rgba(pt, tc->transfer_map[layer],
                                  x, y, TILE_SIZE, TILE_SIZE,
                                  tc->surface->format,
                                  tc->tile->data.color);
            }
            numCleared++;
         }
      }
   }


#if 0
   debug_printf("num cleared: %u\n", numCleared);
#endif
}

static void
sp_flush_tile(struct softpipe_tile_cache* tc, unsigned pos)
{
   int layer = tc->tile_addrs[pos].bits.layer;
   if (!tc->tile_addrs[pos].bits.invalid) {
      if (tc->depth_stencil) {
         pipe_put_tile_raw(tc->transfer[layer], tc->transfer_map[layer],
                           tc->tile_addrs[pos].bits.x * TILE_SIZE,
                           tc->tile_addrs[pos].bits.y * TILE_SIZE,
                           TILE_SIZE, TILE_SIZE,
                           tc->entries[pos]->data.depth32, 0/*STRIDE*/);
      }
      else {
         pipe_put_tile_rgba(tc->transfer[layer], tc->transfer_map[layer],
                            tc->tile_addrs[pos].bits.x * TILE_SIZE,
                            tc->tile_addrs[pos].bits.y * TILE_SIZE,
                            TILE_SIZE, TILE_SIZE,
                            tc->surface->format,
                            tc->entries[pos]->data.color);
      }
      tc->tile_addrs[pos].bits.invalid = 1;  /* mark as empty */
   }
}

/**
 * Flush the tile cache: write all dirty tiles back to the transfer.
 * any tiles "flagged" as cleared will be "really" cleared.
 */
void
sp_flush_tile_cache(struct softpipe_tile_cache *tc)
{
   UNUSED int inuse = 0;
   int i;
   if (tc->num_maps) {
      /* caching a drawing transfer */
      for (int pos = 0; pos < ARRAY_SIZE(tc->entries); pos++) {
         struct softpipe_cached_tile *tile = tc->entries[pos];
         if (!tile)
         {
            assert(tc->tile_addrs[pos].bits.invalid);
            continue;
         }
         sp_flush_tile(tc, pos);
         ++inuse;
      }

      if (!tc->tile)
         tc->tile = sp_alloc_tile(tc);

      for (i = 0; i < tc->num_maps; i++)
         sp_tile_cache_flush_clear(tc, i);
      /* reset all clear flags to zero */
      memset(tc->clear_flags, 0, tc->clear_flags_size);

      tc->last_tile_addr.bits.invalid = 1;
   }

#if 0
   debug_printf("flushed tiles in use: %d\n", inuse);
#endif
}

static struct softpipe_cached_tile *
sp_alloc_tile(struct softpipe_tile_cache *tc)
{
   struct softpipe_cached_tile * tile = MALLOC_STRUCT(softpipe_cached_tile);
   if (!tile)
   {
      /* in this case, steal an existing tile */
      if (!tc->tile)
      {
         unsigned pos;
         for (pos = 0; pos < ARRAY_SIZE(tc->entries); ++pos) {
            if (!tc->entries[pos])
               continue;

            sp_flush_tile(tc, pos);
            tc->tile = tc->entries[pos];
            tc->entries[pos] = NULL;
            break;
         }

         /* this should never happen */
         if (!tc->tile)
            abort();
      }

      tile = tc->tile;
      tc->tile = NULL;

      tc->last_tile_addr.bits.invalid = 1;
   }
   return tile;
}

/**
 * Get a tile from the cache.
 * \param x, y  position of tile, in pixels
 */
struct softpipe_cached_tile *
sp_find_cached_tile(struct softpipe_tile_cache *tc, 
                    union tile_address addr )
{
   struct pipe_transfer *pt;
   /* cache pos/entry: */
   const int pos = CACHE_POS(addr.bits.x,
                             addr.bits.y, addr.bits.layer);
   struct softpipe_cached_tile *tile = tc->entries[pos];
   int layer;
   if (!tile) {
      tile = sp_alloc_tile(tc);
      tc->entries[pos] = tile;
   }

   if (addr.value != tc->tile_addrs[pos].value) {

      layer = tc->tile_addrs[pos].bits.layer;
      if (tc->tile_addrs[pos].bits.invalid == 0) {
         /* put dirty tile back in framebuffer */
         if (tc->depth_stencil) {
            pipe_put_tile_raw(tc->transfer[layer], tc->transfer_map[layer],
                              tc->tile_addrs[pos].bits.x * TILE_SIZE,
                              tc->tile_addrs[pos].bits.y * TILE_SIZE,
                              TILE_SIZE, TILE_SIZE,
                              tile->data.depth32, 0/*STRIDE*/);
         }
         else {
            pipe_put_tile_rgba(tc->transfer[layer], tc->transfer_map[layer],
                               tc->tile_addrs[pos].bits.x * TILE_SIZE,
                               tc->tile_addrs[pos].bits.y * TILE_SIZE,
                               TILE_SIZE, TILE_SIZE,
                               tc->surface->format,
                               tile->data.color);
         }
      }

      tc->tile_addrs[pos] = addr;

      layer = tc->tile_addrs[pos].bits.layer;
      pt = tc->transfer[layer];
      assert(pt->resource);

      if (is_clear_flag_set(tc->clear_flags, addr, tc->clear_flags_size)) {
         /* don't get tile from framebuffer, just clear it */
         if (tc->depth_stencil) {
            clear_tile(tile, pt->resource->format, tc->clear_val);
         }
         else {
            clear_tile_rgba(tile, pt->resource->format, &tc->clear_color);
         }
         clear_clear_flag(tc->clear_flags, addr, tc->clear_flags_size);
      }
      else {
         /* get new tile data from transfer */
         if (tc->depth_stencil) {
            pipe_get_tile_raw(tc->transfer[layer], tc->transfer_map[layer],
                              tc->tile_addrs[pos].bits.x * TILE_SIZE,
                              tc->tile_addrs[pos].bits.y * TILE_SIZE,
                              TILE_SIZE, TILE_SIZE,
                              tile->data.depth32, 0/*STRIDE*/);
         }
         else {
            pipe_get_tile_rgba(tc->transfer[layer], tc->transfer_map[layer],
                               tc->tile_addrs[pos].bits.x * TILE_SIZE,
                               tc->tile_addrs[pos].bits.y * TILE_SIZE,
                               TILE_SIZE, TILE_SIZE,
                               tc->surface->format,
                               tile->data.color);
         }
      }
   }

   tc->last_tile = tile;
   tc->last_tile_addr = addr;
   return tile;
}





/**
 * When a whole surface is being cleared to a value we can avoid
 * fetching tiles above.
 * Save the color and set a 'clearflag' for each tile of the screen.
 */
void
sp_tile_cache_clear(struct softpipe_tile_cache *tc,
                    const union pipe_color_union *color,
                    uint64_t clearValue)
{
   uint pos;

   tc->clear_color = *color;

   tc->clear_val = clearValue;

   /* set flags to indicate all the tiles are cleared */
   memset(tc->clear_flags, 255, tc->clear_flags_size);

   for (pos = 0; pos < ARRAY_SIZE(tc->tile_addrs); pos++) {
      tc->tile_addrs[pos].bits.invalid = 1;
   }
   tc->last_tile_addr.bits.invalid = 1;
}
