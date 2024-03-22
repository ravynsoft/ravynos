/*
 * Copyright (C) 2017-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "xf86drm.h"
#include "drm-uapi/lima_drm.h"

#include "util/u_math.h"
#include "util/ralloc.h"
#include "util/os_time.h"
#include "util/hash_table.h"
#include "util/format/u_format.h"
#include "util/u_upload_mgr.h"
#include "util/u_inlines.h"
#include "util/u_framebuffer.h"

#include "lima_screen.h"
#include "lima_context.h"
#include "lima_job.h"
#include "lima_bo.h"
#include "lima_util.h"
#include "lima_format.h"
#include "lima_resource.h"
#include "lima_texture.h"
#include "lima_fence.h"
#include "lima_gpu.h"
#include "lima_blit.h"

#define VOID2U64(x) ((uint64_t)(unsigned long)(x))

static void
lima_get_fb_info(struct lima_job *job)
{
   struct lima_context *ctx = job->ctx;
   struct lima_job_fb_info *fb = &job->fb;
   struct lima_surface *surf = lima_surface(job->key.cbuf);

   if (!surf)
      surf = lima_surface(job->key.zsbuf);

   if (!surf) {
      /* We don't have neither cbuf nor zsbuf, use dimensions from ctx */
      fb->width = ctx->framebuffer.base.width;
      fb->height =  ctx->framebuffer.base.height;
   } else {
      fb->width = surf->base.width;
      fb->height = surf->base.height;
   }

   int width = align(fb->width, 16) >> 4;
   int height = align(fb->height, 16) >> 4;

   struct lima_screen *screen = lima_screen(ctx->base.screen);

   fb->tiled_w = width;
   fb->tiled_h = height;

   fb->shift_h = 0;
   fb->shift_w = 0;

   int limit = screen->plb_max_blk;
   while ((width * height) > limit ||
          width > PLBU_BLOCK_W_MASK || height > PLBU_BLOCK_H_MASK) {
      if (width >= height || width > PLBU_BLOCK_W_MASK) {
         width = (width + 1) >> 1;
         fb->shift_w++;
      } else {
         height = (height + 1) >> 1;
         fb->shift_h++;
      }
   }

   fb->block_w = width;
   fb->block_h = height;

   fb->shift_min = MIN3(fb->shift_w, fb->shift_h, 2);
}

static struct lima_job *
lima_job_create(struct lima_context *ctx,
                struct pipe_surface *cbuf,
                struct pipe_surface *zsbuf)
{
   struct lima_job *s;

   s = rzalloc(ctx, struct lima_job);
   if (!s)
      return NULL;

   s->fd = lima_screen(ctx->base.screen)->fd;
   s->ctx = ctx;

   s->damage_rect.minx = s->damage_rect.miny = 0xffff;
   s->damage_rect.maxx = s->damage_rect.maxy = 0;
   s->draws = 0;

   s->clear.depth = 0x00ffffff;

   for (int i = 0; i < 2; i++) {
      util_dynarray_init(s->gem_bos + i, s);
      util_dynarray_init(s->bos + i, s);
   }

   util_dynarray_init(&s->vs_cmd_array, s);
   util_dynarray_init(&s->plbu_cmd_array, s);
   util_dynarray_init(&s->plbu_cmd_head, s);

   pipe_surface_reference(&s->key.cbuf, cbuf);
   pipe_surface_reference(&s->key.zsbuf, zsbuf);

   lima_get_fb_info(s);

   s->dump = lima_dump_create();

   return s;
}

static void
lima_job_free(struct lima_job *job)
{
   struct lima_context *ctx = job->ctx;

   _mesa_hash_table_remove_key(ctx->jobs, &job->key);

   if (job->key.cbuf && (job->resolve & PIPE_CLEAR_COLOR0))
      _mesa_hash_table_remove_key(ctx->write_jobs, job->key.cbuf->texture);
   if (job->key.zsbuf && (job->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)))
      _mesa_hash_table_remove_key(ctx->write_jobs, job->key.zsbuf->texture);

   pipe_surface_reference(&job->key.cbuf, NULL);
   pipe_surface_reference(&job->key.zsbuf, NULL);

   lima_dump_free(job->dump);
   job->dump = NULL;

   /* TODO: do we need a cache for job? */
   ralloc_free(job);
}

struct lima_job *
lima_job_get_with_fb(struct lima_context *ctx,
                      struct pipe_surface *cbuf,
                      struct pipe_surface *zsbuf)
{
   struct lima_job_key local_key = {
      .cbuf = cbuf,
      .zsbuf = zsbuf,
   };

   struct hash_entry *entry = _mesa_hash_table_search(ctx->jobs, &local_key);
   if (entry)
      return entry->data;

   struct lima_job *job = lima_job_create(ctx, cbuf, zsbuf);
   if (!job)
      return NULL;

   _mesa_hash_table_insert(ctx->jobs, &job->key, job);

   return job;
}

static struct lima_job *
_lima_job_get(struct lima_context *ctx)
{
   struct lima_context_framebuffer *fb = &ctx->framebuffer;

   return lima_job_get_with_fb(ctx, fb->base.cbufs[0], fb->base.zsbuf);
}

/*
 * Note: this function can only be called in draw code path,
 * must not exist in flush code path.
 */
struct lima_job *
lima_job_get(struct lima_context *ctx)
{
   if (ctx->job)
      return ctx->job;

   ctx->job = _lima_job_get(ctx);
   return ctx->job;
}

bool lima_job_add_bo(struct lima_job *job, int pipe,
                     struct lima_bo *bo, uint32_t flags)
{
   util_dynarray_foreach(job->gem_bos + pipe, struct drm_lima_gem_submit_bo, gem_bo) {
      if (bo->handle == gem_bo->handle) {
         gem_bo->flags |= flags;
         return true;
      }
   }

   struct drm_lima_gem_submit_bo *job_bo =
      util_dynarray_grow(job->gem_bos + pipe, struct drm_lima_gem_submit_bo, 1);
   job_bo->handle = bo->handle;
   job_bo->flags = flags;

   struct lima_bo **jbo = util_dynarray_grow(job->bos + pipe, struct lima_bo *, 1);
   *jbo = bo;

   /* prevent bo from being freed when job start */
   lima_bo_reference(bo);

   return true;
}

static bool
lima_job_start(struct lima_job *job, int pipe, void *frame, uint32_t size)
{
   struct lima_context *ctx = job->ctx;
   struct drm_lima_gem_submit req = {
      .ctx = ctx->id,
      .pipe = pipe,
      .nr_bos = job->gem_bos[pipe].size / sizeof(struct drm_lima_gem_submit_bo),
      .bos = VOID2U64(util_dynarray_begin(job->gem_bos + pipe)),
      .frame = VOID2U64(frame),
      .frame_size = size,
      .out_sync = ctx->out_sync[pipe],
   };

   if (ctx->in_sync_fd >= 0) {
      int err = drmSyncobjImportSyncFile(job->fd, ctx->in_sync[pipe],
                                         ctx->in_sync_fd);
      if (err)
         return false;

      req.in_sync[0] = ctx->in_sync[pipe];
      close(ctx->in_sync_fd);
      ctx->in_sync_fd = -1;
   }

   bool ret = drmIoctl(job->fd, DRM_IOCTL_LIMA_GEM_SUBMIT, &req) == 0;

   util_dynarray_foreach(job->bos + pipe, struct lima_bo *, bo) {
      lima_bo_unreference(*bo);
   }

   return ret;
}

static bool
lima_job_wait(struct lima_job *job, int pipe, uint64_t timeout_ns)
{
   int64_t abs_timeout = os_time_get_absolute_timeout(timeout_ns);
   if (abs_timeout == OS_TIMEOUT_INFINITE)
      abs_timeout = INT64_MAX;

   struct lima_context *ctx = job->ctx;
   return !drmSyncobjWait(job->fd, ctx->out_sync + pipe, 1, abs_timeout, 0, NULL);
}

static bool
lima_job_has_bo(struct lima_job *job, struct lima_bo *bo, bool all)
{
   for (int i = 0; i < 2; i++) {
      util_dynarray_foreach(job->gem_bos + i, struct drm_lima_gem_submit_bo, gem_bo) {
         if (bo->handle == gem_bo->handle) {
            if (all || gem_bo->flags & LIMA_SUBMIT_BO_WRITE)
               return true;
            else
               break;
         }
      }
   }

   return false;
}

void *
lima_job_create_stream_bo(struct lima_job *job, int pipe,
                          unsigned size, uint32_t *va)
{
   struct lima_context *ctx = job->ctx;

   void *cpu;
   unsigned offset;
   struct pipe_resource *pres = NULL;
   u_upload_alloc(ctx->uploader, 0, size, 0x40, &offset, &pres, &cpu);

   struct lima_resource *res = lima_resource(pres);
   *va = res->bo->va + offset;

   lima_job_add_bo(job, pipe, res->bo, LIMA_SUBMIT_BO_READ);

   pipe_resource_reference(&pres, NULL);

   return cpu;
}

static inline struct lima_damage_region *
lima_job_get_damage(struct lima_job *job)
{
   if (!(job->key.cbuf && (job->resolve & PIPE_CLEAR_COLOR0)))
      return NULL;

   struct lima_surface *surf = lima_surface(job->key.cbuf);
   struct lima_resource *res = lima_resource(surf->base.texture);
   return &res->damage;
}

static bool
lima_fb_cbuf_needs_reload(struct lima_job *job)
{
   if (!job->key.cbuf)
      return false;

   struct lima_surface *surf = lima_surface(job->key.cbuf);
   struct lima_resource *res = lima_resource(surf->base.texture);
   if (res->damage.region) {
      /* for EGL_KHR_partial_update, when EGL_EXT_buffer_age is enabled,
       * we need to reload damage region, otherwise just want to reload
       * the region not aligned to tile boundary */
      //if (!res->damage.aligned)
      //   return true;
      return true;
   }
   else if (surf->reload & PIPE_CLEAR_COLOR0)
         return true;

   return false;
}

static bool
lima_fb_zsbuf_needs_reload(struct lima_job *job)
{
   if (!job->key.zsbuf)
      return false;

   struct lima_surface *surf = lima_surface(job->key.zsbuf);
   if (surf->reload & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL))
         return true;

   return false;
}

static void
lima_pack_reload_plbu_cmd(struct lima_job *job, struct pipe_surface *psurf)
{
   struct lima_job_fb_info *fb = &job->fb;
   struct lima_context *ctx = job->ctx;
   struct pipe_box src = {
      .x = 0,
      .y = 0,
      .width = fb->width,
      .height = fb->height,
   };

   struct pipe_box dst = {
      .x = 0,
      .y = 0,
      .width = fb->width,
      .height = fb->height,
   };

   if (ctx->framebuffer.base.samples > 1) {
      for (int i = 0; i < LIMA_MAX_SAMPLES; i++) {
         lima_pack_blit_cmd(job, &job->plbu_cmd_head,
                            psurf, &src, &dst,
                            PIPE_TEX_FILTER_NEAREST, false,
                            (1 << i), i);
      }
   } else {
      lima_pack_blit_cmd(job, &job->plbu_cmd_head,
                         psurf, &src, &dst,
                         PIPE_TEX_FILTER_NEAREST, false,
                         0xf, 0);
   }
}

static void
lima_pack_head_plbu_cmd(struct lima_job *job)
{
   struct lima_context *ctx = job->ctx;
   struct lima_job_fb_info *fb = &job->fb;

   PLBU_CMD_BEGIN(&job->plbu_cmd_head, 10);

   assert((fb->block_w & PLBU_BLOCK_W_MASK) == fb->block_w);
   assert((fb->block_h & PLBU_BLOCK_H_MASK) == fb->block_h);

   PLBU_CMD_UNKNOWN2();
   PLBU_CMD_BLOCK_STEP(fb->shift_min, fb->shift_h, fb->shift_w);
   PLBU_CMD_TILED_DIMENSIONS(fb->tiled_w, fb->tiled_h);
   PLBU_CMD_BLOCK_STRIDE(fb->block_w);

   PLBU_CMD_ARRAY_ADDRESS(
      ctx->plb_gp_stream->va + ctx->plb_index * ctx->plb_gp_size,
      fb->block_w * fb->block_h);

   PLBU_CMD_END();

   if (lima_fb_cbuf_needs_reload(job)) {
      lima_pack_reload_plbu_cmd(job, job->key.cbuf);
   }

   if (lima_fb_zsbuf_needs_reload(job))
      lima_pack_reload_plbu_cmd(job, job->key.zsbuf);
}

static void
hilbert_rotate(int n, int *x, int *y, int rx, int ry)
{
   if (ry == 0) {
      if (rx == 1) {
         *x = n-1 - *x;
         *y = n-1 - *y;
      }

      /* Swap x and y */
      int t  = *x;
      *x = *y;
      *y = t;
   }
}

static void
hilbert_coords(int n, int d, int *x, int *y)
{
   int rx, ry, i, t=d;

   *x = *y = 0;

   for (i = 0; (1 << i) < n; i++) {

      rx = 1 & (t / 2);
      ry = 1 & (t ^ rx);

      hilbert_rotate(1 << i, x, y, rx, ry);

      *x += rx << i;
      *y += ry << i;

      t /= 4;
   }
}

static int
lima_get_pp_stream_size(int num_pp, int tiled_w, int tiled_h, uint32_t *off)
{
   /* carefully calculate each stream start address:
    * 1. overflow: each stream size may be different due to
    *    fb->tiled_w * fb->tiled_h can't be divided by num_pp,
    *    extra size should be added to the preceeding stream
    * 2. alignment: each stream address should be 0x20 aligned
    */
   int delta = tiled_w * tiled_h / num_pp * 16 + 16;
   int remain = tiled_w * tiled_h % num_pp;
   int offset = 0;

   for (int i = 0; i < num_pp; i++) {
      off[i] = offset;

      offset += delta;
      if (remain) {
         offset += 16;
         remain--;
      }
      offset = align(offset, 0x20);
   }

   return offset;
}

static void
lima_generate_pp_stream(struct lima_job *job, int off_x, int off_y,
                        int tiled_w, int tiled_h)
{
   struct lima_context *ctx = job->ctx;
   struct lima_pp_stream_state *ps = &ctx->pp_stream;
   struct lima_job_fb_info *fb = &job->fb;
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   int num_pp = screen->num_pp;
   assert(num_pp > 0);

   /* use hilbert_coords to generates 1D to 2D relationship.
    * 1D for pp stream index and 2D for plb block x/y on framebuffer.
    * if multi pp, interleave the 1D index to make each pp's render target
    * close enough which should result close workload
    */
   int max = MAX2(tiled_w, tiled_h);
   int index = 0;
   uint32_t *stream[8];
   int si[8] = {0};
   int dim = 0;
   int count = 0;

   /* Don't update count if we get zero rect. We'll just generate
    * PP stream with just terminators in it.
    */
   if ((tiled_w * tiled_h) != 0) {
      dim = util_logbase2_ceil(max);
      count = 1 << (dim + dim);
   }

   for (int i = 0; i < num_pp; i++)
      stream[i] = ps->map + ps->offset[i];

   for (int i = 0; i < count; i++) {
      int x, y;
      hilbert_coords(max, i, &x, &y);
      if (x < tiled_w && y < tiled_h) {
         x += off_x;
         y += off_y;

         int pp = index % num_pp;
         int offset = ((y >> fb->shift_h) * fb->block_w +
                       (x >> fb->shift_w)) * LIMA_CTX_PLB_BLK_SIZE;
         int plb_va = ctx->plb[ctx->plb_index]->va + offset;

         stream[pp][si[pp]++] = 0;
         stream[pp][si[pp]++] = 0xB8000000 | x | (y << 8);
         stream[pp][si[pp]++] = 0xE0000002 | ((plb_va >> 3) & ~0xE0000003);
         stream[pp][si[pp]++] = 0xB0000000;

         index++;
      }
   }

   for (int i = 0; i < num_pp; i++) {
      stream[i][si[i]++] = 0;
      stream[i][si[i]++] = 0xBC000000;
      stream[i][si[i]++] = 0;
      stream[i][si[i]++] = 0;

      lima_dump_command_stream_print(
         job->dump, stream[i], si[i] * 4,
         false, "pp plb stream %d at va %x\n",
         i, ps->va + ps->offset[i]);
   }
}

static void
lima_free_stale_pp_stream_bo(struct lima_context *ctx)
{
   list_for_each_entry_safe(struct lima_ctx_plb_pp_stream, entry,
                            &ctx->plb_pp_stream_lru_list, lru_list) {
      if (ctx->plb_stream_cache_size <= lima_plb_pp_stream_cache_size)
         break;

      struct hash_entry *hash_entry =
         _mesa_hash_table_search(ctx->plb_pp_stream, &entry->key);
      if (hash_entry)
         _mesa_hash_table_remove(ctx->plb_pp_stream, hash_entry);
      list_del(&entry->lru_list);

      ctx->plb_stream_cache_size -= entry->bo->size;
      lima_bo_unreference(entry->bo);

      ralloc_free(entry);
   }
}

static void
lima_update_damage_pp_stream(struct lima_job *job)
{
   struct lima_context *ctx = job->ctx;
   struct lima_damage_region *ds = lima_job_get_damage(job);
   struct lima_job_fb_info *fb = &job->fb;
   struct pipe_scissor_state bound;
   struct pipe_scissor_state *dr = &job->damage_rect;

   if (ds && ds->region) {
      struct pipe_scissor_state *dbound = &ds->bound;
      bound.minx = MAX2(dbound->minx, dr->minx >> 4);
      bound.miny = MAX2(dbound->miny, dr->miny >> 4);
      bound.maxx = MIN2(dbound->maxx, (dr->maxx + 0xf) >> 4);
      bound.maxy = MIN2(dbound->maxy, (dr->maxy + 0xf) >> 4);
   } else {
      bound.minx = dr->minx >> 4;
      bound.miny = dr->miny >> 4;
      bound.maxx = (dr->maxx + 0xf) >> 4;
      bound.maxy = (dr->maxy + 0xf) >> 4;
   }

   /* Clamp to FB size */
   bound.minx = MIN2(bound.minx, fb->tiled_w);
   bound.miny = MIN2(bound.miny, fb->tiled_h);
   bound.maxx = MIN2(bound.maxx, fb->tiled_w);
   bound.maxy = MIN2(bound.maxy, fb->tiled_h);

   struct lima_ctx_plb_pp_stream_key key = {
      .plb_index = ctx->plb_index,
      .minx = bound.minx,
      .miny = bound.miny,
      .maxx = bound.maxx,
      .maxy = bound.maxy,
      .shift_w = fb->shift_w,
      .shift_h = fb->shift_h,
      .block_w = fb->block_w,
      .block_h = fb->block_h,
   };

   struct hash_entry *entry =
      _mesa_hash_table_search(ctx->plb_pp_stream, &key);
   if (entry) {
      struct lima_ctx_plb_pp_stream *s = entry->data;

      list_del(&s->lru_list);
      list_addtail(&s->lru_list, &ctx->plb_pp_stream_lru_list);

      ctx->pp_stream.map = lima_bo_map(s->bo);
      ctx->pp_stream.va = s->bo->va;
      memcpy(ctx->pp_stream.offset, s->offset, sizeof(s->offset));

      lima_job_add_bo(job, LIMA_PIPE_PP, s->bo, LIMA_SUBMIT_BO_READ);

      return;
   }

   lima_free_stale_pp_stream_bo(ctx);

   struct lima_screen *screen = lima_screen(ctx->base.screen);
   struct lima_ctx_plb_pp_stream *s =
      rzalloc(ctx->plb_pp_stream, struct lima_ctx_plb_pp_stream);

   list_inithead(&s->lru_list);
   s->key.plb_index = ctx->plb_index;
   s->key.minx = bound.minx;
   s->key.maxx = bound.maxx;
   s->key.miny = bound.miny;
   s->key.maxy = bound.maxy;
   s->key.shift_w = fb->shift_w;
   s->key.shift_h = fb->shift_h;
   s->key.block_w = fb->block_w;
   s->key.block_h = fb->block_h;

   int tiled_w = bound.maxx - bound.minx;
   int tiled_h = bound.maxy - bound.miny;
   int size = lima_get_pp_stream_size(
      screen->num_pp, tiled_w, tiled_h, s->offset);

   s->bo = lima_bo_create(screen, size, 0);

   ctx->pp_stream.map = lima_bo_map(s->bo);
   ctx->pp_stream.va = s->bo->va;
   memcpy(ctx->pp_stream.offset, s->offset, sizeof(s->offset));

   lima_generate_pp_stream(job, bound.minx, bound.miny, tiled_w, tiled_h);

   ctx->plb_stream_cache_size += size;
   list_addtail(&s->lru_list, &ctx->plb_pp_stream_lru_list);
   _mesa_hash_table_insert(ctx->plb_pp_stream, &s->key, s);

   lima_job_add_bo(job, LIMA_PIPE_PP, s->bo, LIMA_SUBMIT_BO_READ);
}

static bool
lima_damage_fullscreen(struct lima_job *job)
{
   struct pipe_scissor_state *dr = &job->damage_rect;

   return dr->minx == 0 &&
          dr->miny == 0 &&
          dr->maxx == job->fb.width &&
          dr->maxy == job->fb.height;
}

static void
lima_update_pp_stream(struct lima_job *job)
{
   struct lima_context *ctx = job->ctx;
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   struct lima_damage_region *damage = lima_job_get_damage(job);
   if ((screen->gpu_type == DRM_LIMA_PARAM_GPU_ID_MALI400) ||
       (damage && damage->region) || !lima_damage_fullscreen(job))
      lima_update_damage_pp_stream(job);
   else
      /* Mali450 doesn't need full PP stream */
      ctx->pp_stream.map = NULL;
}

static void
lima_update_job_bo(struct lima_job *job)
{
   struct lima_context *ctx = job->ctx;

   lima_job_add_bo(job, LIMA_PIPE_GP, ctx->plb_gp_stream,
                      LIMA_SUBMIT_BO_READ);
   lima_job_add_bo(job, LIMA_PIPE_GP, ctx->plb[ctx->plb_index],
                      LIMA_SUBMIT_BO_WRITE);
   lima_job_add_bo(job, LIMA_PIPE_GP, ctx->gp_tile_heap[ctx->plb_index],
                      LIMA_SUBMIT_BO_WRITE);

   lima_dump_command_stream_print(
      job->dump, ctx->plb_gp_stream->map + ctx->plb_index * ctx->plb_gp_size,
      ctx->plb_gp_size, false, "gp plb stream at va %x\n",
      ctx->plb_gp_stream->va + ctx->plb_index * ctx->plb_gp_size);

   lima_job_add_bo(job, LIMA_PIPE_PP, ctx->plb[ctx->plb_index],
                      LIMA_SUBMIT_BO_READ);
   lima_job_add_bo(job, LIMA_PIPE_PP, ctx->gp_tile_heap[ctx->plb_index],
                      LIMA_SUBMIT_BO_READ);

   struct lima_screen *screen = lima_screen(ctx->base.screen);
   lima_job_add_bo(job, LIMA_PIPE_PP, screen->pp_buffer, LIMA_SUBMIT_BO_READ);
}

static void
lima_finish_plbu_cmd(struct util_dynarray *plbu_cmd_array)
{
   int i = 0;
   uint32_t *plbu_cmd = util_dynarray_ensure_cap(plbu_cmd_array, plbu_cmd_array->size + 2 * 4);

   plbu_cmd[i++] = 0x00000000;
   plbu_cmd[i++] = 0x50000000; /* END */

   plbu_cmd_array->size += i * 4;
}

static void
lima_pack_wb_zsbuf_reg(struct lima_job *job, uint32_t *wb_reg, int wb_idx)
{
   struct lima_job_fb_info *fb = &job->fb;
   struct pipe_surface *zsbuf = job->key.zsbuf;
   struct lima_resource *res = lima_resource(zsbuf->texture);
   int level = zsbuf->u.tex.level;
   uint32_t format = lima_format_get_pixel(zsbuf->format);

   struct lima_pp_wb_reg *wb = (void *)wb_reg;
   wb[wb_idx].type = 0x01; /* 1 for depth, stencil */
   wb[wb_idx].address = res->bo->va + res->levels[level].offset;
   wb[wb_idx].pixel_format = format;
   if (res->tiled) {
      wb[wb_idx].pixel_layout = 0x2;
      wb[wb_idx].pitch = fb->tiled_w;
   } else {
      wb[wb_idx].pixel_layout = 0x0;
      wb[wb_idx].pitch = res->levels[level].stride / 8;
   }
   wb[wb_idx].flags = 0;
   unsigned nr_samples = zsbuf->nr_samples ?
                         zsbuf->nr_samples : MAX2(1, zsbuf->texture->nr_samples);
   if (nr_samples > 1) {
      wb[wb_idx].mrt_pitch = res->mrt_pitch;
      wb[wb_idx].mrt_bits = u_bit_consecutive(0, nr_samples);
   }
}

static void
lima_pack_wb_cbuf_reg(struct lima_job *job, uint32_t *frame_reg,
                      uint32_t *wb_reg, int wb_idx)
{
   struct lima_job_fb_info *fb = &job->fb;
   struct pipe_surface *cbuf = job->key.cbuf;
   struct lima_resource *res = lima_resource(cbuf->texture);
   int level = cbuf->u.tex.level;
   unsigned layer = cbuf->u.tex.first_layer;
   uint32_t format = lima_format_get_pixel(cbuf->format);
   bool swap_channels = lima_format_get_pixel_swap_rb(cbuf->format);

   struct lima_pp_frame_reg *frame = (void *)frame_reg;
   frame->channel_layout = lima_format_get_channel_layout(cbuf->format);

   struct lima_pp_wb_reg *wb = (void *)wb_reg;
   wb[wb_idx].type = 0x02; /* 2 for color buffer */
   wb[wb_idx].address = res->bo->va + res->levels[level].offset + layer * res->levels[level].layer_stride;
   wb[wb_idx].pixel_format = format;
   if (res->tiled) {
      wb[wb_idx].pixel_layout = 0x2;
      wb[wb_idx].pitch = fb->tiled_w;
   } else {
      wb[wb_idx].pixel_layout = 0x0;
      wb[wb_idx].pitch = res->levels[level].stride / 8;
   }
   wb[wb_idx].flags = swap_channels ? 0x4 : 0x0;
   unsigned nr_samples = cbuf->nr_samples ?
                         cbuf->nr_samples : MAX2(1, cbuf->texture->nr_samples);
   if (nr_samples > 1) {
      wb[wb_idx].mrt_pitch = res->mrt_pitch;
      wb[wb_idx].mrt_bits = u_bit_consecutive(0, nr_samples);
   }
}

static void
lima_pack_pp_frame_reg(struct lima_job *job, uint32_t *frame_reg,
                       uint32_t *wb_reg)
{
   struct lima_context *ctx = job->ctx;
   struct lima_job_fb_info *fb = &job->fb;
   struct pipe_surface *cbuf = job->key.cbuf;
   struct lima_pp_frame_reg *frame = (void *)frame_reg;
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   int wb_idx = 0;

   frame->render_address = screen->pp_buffer->va + pp_frame_rsw_offset;
   frame->flags = 0x02;
   if (cbuf && util_format_is_float(cbuf->format)) {
      frame->flags |= 0x01; /* enable fp16 */
      frame->clear_value_color   = (uint32_t)(job->clear.color_16pc & 0xffffffffUL);
      frame->clear_value_color_1 = (uint32_t)(job->clear.color_16pc >> 32);
      frame->clear_value_color_2 = 0;
      frame->clear_value_color_3 = 0;
   }
   else {
      frame->clear_value_color   = job->clear.color_8pc;
      frame->clear_value_color_1 = job->clear.color_8pc;
      frame->clear_value_color_2 = job->clear.color_8pc;
      frame->clear_value_color_3 = job->clear.color_8pc;
   }

   frame->clear_value_depth = job->clear.depth;
   frame->clear_value_stencil = job->clear.stencil;
   frame->one = 1;

   frame->width = fb->width - 1;
   frame->height = fb->height - 1;

   /* frame->fragment_stack_address is overwritten per-pp in the kernel
    * by the values of pp_frame.fragment_stack_address[i] */

   /* These are "stack size" and "stack offset" shifted,
    * here they are assumed to be always the same. */
   frame->fragment_stack_size = job->pp_max_stack_size << 16 | job->pp_max_stack_size;

   /* related with MSAA and different value when r4p0/r7p0 */
   frame->supersampled_height = fb->height * 2 - 1;
   frame->scale = 0xE0C;

   frame->dubya = 0x77;
   frame->onscreen = 1;
   frame->blocking = (fb->shift_min << 28) | (fb->shift_h << 16) | fb->shift_w;

   /* Set default layout to 8888 */
   frame->channel_layout = 0x8888;

   if (cbuf && (job->resolve & PIPE_CLEAR_COLOR0))
      lima_pack_wb_cbuf_reg(job, frame_reg, wb_reg, wb_idx++);

   if (job->key.zsbuf &&
       (job->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)))
      lima_pack_wb_zsbuf_reg(job, wb_reg, wb_idx++);
}

void
lima_do_job(struct lima_job *job)
{
   #define pp_stack_pp_size 0x400

   struct lima_context *ctx = job->ctx;

   lima_pack_head_plbu_cmd(job);
   lima_finish_plbu_cmd(&job->plbu_cmd_array);

   lima_update_job_bo(job);

   int vs_cmd_size = job->vs_cmd_array.size;
   uint32_t vs_cmd_va = 0;

   if (vs_cmd_size) {
      void *vs_cmd = lima_job_create_stream_bo(
         job, LIMA_PIPE_GP, vs_cmd_size, &vs_cmd_va);
      memcpy(vs_cmd, util_dynarray_begin(&job->vs_cmd_array), vs_cmd_size);

      lima_dump_command_stream_print(
         job->dump, vs_cmd, vs_cmd_size, false, "flush vs cmd at va %x\n", vs_cmd_va);
      lima_dump_vs_command_stream_print(job->dump, vs_cmd, vs_cmd_size, vs_cmd_va);
   }

   uint32_t plbu_cmd_va;
   int plbu_cmd_size = job->plbu_cmd_array.size + job->plbu_cmd_head.size;
   void *plbu_cmd = lima_job_create_stream_bo(
      job, LIMA_PIPE_GP, plbu_cmd_size, &plbu_cmd_va);
   memcpy(plbu_cmd,
          util_dynarray_begin(&job->plbu_cmd_head),
          job->plbu_cmd_head.size);
   memcpy(plbu_cmd + job->plbu_cmd_head.size,
          util_dynarray_begin(&job->plbu_cmd_array),
          job->plbu_cmd_array.size);

   lima_dump_command_stream_print(
      job->dump, plbu_cmd, plbu_cmd_size, false, "flush plbu cmd at va %x\n", plbu_cmd_va);
   lima_dump_plbu_command_stream_print(job->dump, plbu_cmd, plbu_cmd_size, plbu_cmd_va);

   struct lima_screen *screen = lima_screen(ctx->base.screen);
   struct drm_lima_gp_frame gp_frame;
   struct lima_gp_frame_reg *gp_frame_reg = (void *)gp_frame.frame;
   gp_frame_reg->vs_cmd_start = vs_cmd_va;
   gp_frame_reg->vs_cmd_end = vs_cmd_va + vs_cmd_size;
   gp_frame_reg->plbu_cmd_start = plbu_cmd_va;
   gp_frame_reg->plbu_cmd_end = plbu_cmd_va + plbu_cmd_size;
   gp_frame_reg->tile_heap_start = ctx->gp_tile_heap[ctx->plb_index]->va;
   gp_frame_reg->tile_heap_end = ctx->gp_tile_heap[ctx->plb_index]->va + ctx->gp_tile_heap_size;

   lima_dump_command_stream_print(
      job->dump, &gp_frame, sizeof(gp_frame), false, "add gp frame\n");

   if (!lima_job_start(job, LIMA_PIPE_GP, &gp_frame, sizeof(gp_frame)))
      fprintf(stderr, "gp job error\n");

   if (job->dump) {
      if (lima_job_wait(job, LIMA_PIPE_GP, OS_TIMEOUT_INFINITE)) {
         if (ctx->gp_output) {
            float *pos = lima_bo_map(ctx->gp_output);
            lima_dump_command_stream_print(
               job->dump, pos, 4 * 4 * 16, true, "gl_pos dump at va %x\n",
               ctx->gp_output->va);
         }

         uint32_t *plb = lima_bo_map(ctx->plb[ctx->plb_index]);
         lima_dump_command_stream_print(
            job->dump, plb, LIMA_CTX_PLB_BLK_SIZE, false, "plb dump at va %x\n",
            ctx->plb[ctx->plb_index]->va);
      }
      else {
         fprintf(stderr, "gp job wait error\n");
         exit(1);
      }
   }

   uint32_t pp_stack_va = 0;
   if (job->pp_max_stack_size) {
      lima_job_create_stream_bo(
         job, LIMA_PIPE_PP,
         screen->num_pp * job->pp_max_stack_size * pp_stack_pp_size,
         &pp_stack_va);
   }

   lima_update_pp_stream(job);

   struct lima_pp_stream_state *ps = &ctx->pp_stream;
   if (screen->gpu_type == DRM_LIMA_PARAM_GPU_ID_MALI400) {
      struct drm_lima_m400_pp_frame pp_frame = {0};
      lima_pack_pp_frame_reg(job, pp_frame.frame, pp_frame.wb);
      pp_frame.num_pp = screen->num_pp;

      for (int i = 0; i < screen->num_pp; i++) {
         pp_frame.plbu_array_address[i] = ps->va + ps->offset[i];
         if (job->pp_max_stack_size)
            pp_frame.fragment_stack_address[i] = pp_stack_va +
               job->pp_max_stack_size * pp_stack_pp_size * i;
      }

      lima_dump_command_stream_print(
         job->dump, &pp_frame, sizeof(pp_frame), false, "add pp frame\n");

      if (!lima_job_start(job, LIMA_PIPE_PP, &pp_frame, sizeof(pp_frame)))
         fprintf(stderr, "pp job error\n");
   }
   else {
      struct drm_lima_m450_pp_frame pp_frame = {0};
      lima_pack_pp_frame_reg(job, pp_frame.frame, pp_frame.wb);
      pp_frame.num_pp = screen->num_pp;

      if (job->pp_max_stack_size)
         for (int i = 0; i < screen->num_pp; i++)
            pp_frame.fragment_stack_address[i] = pp_stack_va +
               job->pp_max_stack_size * pp_stack_pp_size * i;

      if (ps->map) {
         for (int i = 0; i < screen->num_pp; i++)
            pp_frame.plbu_array_address[i] = ps->va + ps->offset[i];
      }
      else {
         pp_frame.use_dlbu = true;

         struct lima_job_fb_info *fb = &job->fb;
         pp_frame.dlbu_regs[0] = ctx->plb[ctx->plb_index]->va;
         pp_frame.dlbu_regs[1] = ((fb->tiled_h - 1) << 16) | (fb->tiled_w - 1);
         unsigned s = util_logbase2(LIMA_CTX_PLB_BLK_SIZE) - 7;
         pp_frame.dlbu_regs[2] = (s << 28) | (fb->shift_h << 16) | fb->shift_w;
         pp_frame.dlbu_regs[3] = ((fb->tiled_h - 1) << 24) | ((fb->tiled_w - 1) << 16);
      }

      lima_dump_command_stream_print(
         job->dump, &pp_frame, sizeof(pp_frame), false, "add pp frame\n");

      if (!lima_job_start(job, LIMA_PIPE_PP, &pp_frame, sizeof(pp_frame)))
         fprintf(stderr, "pp job error\n");
   }

   if (job->dump) {
      if (!lima_job_wait(job, LIMA_PIPE_PP, OS_TIMEOUT_INFINITE)) {
         fprintf(stderr, "pp wait error\n");
         exit(1);
      }
   }

   ctx->plb_index = (ctx->plb_index + 1) % lima_ctx_num_plb;

   /* Set reload flags for next draw. It'll be unset if buffer is cleared */
   if (job->key.cbuf && (job->resolve & PIPE_CLEAR_COLOR0)) {
      struct lima_surface *surf = lima_surface(job->key.cbuf);
      surf->reload = PIPE_CLEAR_COLOR0;
   }

   if (job->key.zsbuf && (job->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL))) {
      struct lima_surface *surf = lima_surface(job->key.zsbuf);
      surf->reload = (job->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL));
   }

   if (ctx->job == job)
      ctx->job = NULL;

   lima_job_free(job);
}

void
lima_flush(struct lima_context *ctx)
{
   hash_table_foreach(ctx->jobs, entry) {
      struct lima_job *job = entry->data;
      lima_do_job(job);
   }
}

void
lima_flush_job_accessing_bo(
   struct lima_context *ctx, struct lima_bo *bo, bool write)
{
   hash_table_foreach(ctx->jobs, entry) {
      struct lima_job *job = entry->data;
      if (lima_job_has_bo(job, bo, write))
         lima_do_job(job);
   }
}

/*
 * This is for current job flush previous job which write to the resource it wants
 * to read. Tipical usage is flush the FBO which is used as current task's texture.
 */
void
lima_flush_previous_job_writing_resource(
   struct lima_context *ctx, struct pipe_resource *prsc)
{
   struct hash_entry *entry = _mesa_hash_table_search(ctx->write_jobs, prsc);

   if (entry) {
      struct lima_job *job = entry->data;

      /* do not flush current job */
      if (job != ctx->job)
         lima_do_job(job);
   }
}

static void
lima_pipe_flush(struct pipe_context *pctx, struct pipe_fence_handle **fence,
                unsigned flags)
{
   struct lima_context *ctx = lima_context(pctx);

   lima_flush(ctx);

   if (fence) {
      int drm_fd = lima_screen(ctx->base.screen)->fd;
      int fd;

      if (!drmSyncobjExportSyncFile(drm_fd, ctx->out_sync[LIMA_PIPE_PP], &fd))
         *fence = lima_fence_create(fd);
   }
}

static void
lima_texture_barrier(struct pipe_context *pctx, unsigned flags)
{
    struct lima_context *ctx = lima_context(pctx);

    lima_flush(ctx);
}

static bool
lima_job_compare(const void *s1, const void *s2)
{
   return memcmp(s1, s2, sizeof(struct lima_job_key)) == 0;
}

static uint32_t
lima_job_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct lima_job_key));
}

bool lima_job_init(struct lima_context *ctx)
{
   int fd = lima_screen(ctx->base.screen)->fd;

   ctx->jobs = _mesa_hash_table_create(ctx, lima_job_hash, lima_job_compare);
   if (!ctx->jobs)
      return false;

   ctx->write_jobs = _mesa_hash_table_create(
      ctx, _mesa_hash_pointer, _mesa_key_pointer_equal);
   if (!ctx->write_jobs)
      return false;

   ctx->in_sync_fd = -1;

   for (int i = 0; i < 2; i++) {
      if (drmSyncobjCreate(fd, DRM_SYNCOBJ_CREATE_SIGNALED, ctx->in_sync + i) ||
          drmSyncobjCreate(fd, DRM_SYNCOBJ_CREATE_SIGNALED, ctx->out_sync + i))
         return false;
   }

   ctx->base.flush = lima_pipe_flush;
   ctx->base.texture_barrier = lima_texture_barrier;

   return true;
}

void lima_job_fini(struct lima_context *ctx)
{
   int fd = lima_screen(ctx->base.screen)->fd;

   lima_flush(ctx);

   for (int i = 0; i < 2; i++) {
      if (ctx->in_sync[i])
         drmSyncobjDestroy(fd, ctx->in_sync[i]);
      if (ctx->out_sync[i])
         drmSyncobjDestroy(fd, ctx->out_sync[i]);
   }

   if (ctx->in_sync_fd >= 0)
      close(ctx->in_sync_fd);
}
