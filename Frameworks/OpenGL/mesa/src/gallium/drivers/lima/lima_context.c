/*
 * Copyright (c) 2017-2019 Lima Project
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
 *
 */

#include "util/u_memory.h"
#include "util/u_blitter.h"
#include "util/u_upload_mgr.h"
#include "util/u_math.h"
#include "util/u_debug.h"
#include "util/ralloc.h"
#include "util/u_inlines.h"
#include "util/u_debug_cb.h"
#include "util/hash_table.h"

#include "lima_screen.h"
#include "lima_context.h"
#include "lima_resource.h"
#include "lima_bo.h"
#include "lima_job.h"
#include "lima_util.h"
#include "lima_fence.h"

#include <drm-uapi/lima_drm.h>
#include <xf86drm.h>

int lima_ctx_num_plb = LIMA_CTX_PLB_DEF_NUM;

uint32_t
lima_ctx_buff_va(struct lima_context *ctx, enum lima_ctx_buff buff)
{
   struct lima_job *job = lima_job_get(ctx);
   struct lima_ctx_buff_state *cbs = ctx->buffer_state + buff;
   struct lima_resource *res = lima_resource(cbs->res);
   int pipe = buff < lima_ctx_buff_num_gp ? LIMA_PIPE_GP : LIMA_PIPE_PP;

   lima_job_add_bo(job, pipe, res->bo, LIMA_SUBMIT_BO_READ);

   return res->bo->va + cbs->offset;
}

void *
lima_ctx_buff_map(struct lima_context *ctx, enum lima_ctx_buff buff)
{
   struct lima_ctx_buff_state *cbs = ctx->buffer_state + buff;
   struct lima_resource *res = lima_resource(cbs->res);

   return lima_bo_map(res->bo) + cbs->offset;
}

void *
lima_ctx_buff_alloc(struct lima_context *ctx, enum lima_ctx_buff buff,
                    unsigned size)
{
   struct lima_ctx_buff_state *cbs = ctx->buffer_state + buff;
   void *ret = NULL;

   cbs->size = align(size, 0x40);

   u_upload_alloc(ctx->uploader, 0, cbs->size, 0x40, &cbs->offset,
                  &cbs->res, &ret);

   return ret;
}

static int
lima_context_create_drm_ctx(struct lima_screen *screen)
{
   struct drm_lima_ctx_create req = {0};

   int ret = drmIoctl(screen->fd, DRM_IOCTL_LIMA_CTX_CREATE, &req);
   if (ret)
      return errno;

   return req.id;
}

static void
lima_context_free_drm_ctx(struct lima_screen *screen, int id)
{
   struct drm_lima_ctx_free req = {
      .id = id,
   };

   drmIoctl(screen->fd, DRM_IOCTL_LIMA_CTX_FREE, &req);
}

static void
lima_invalidate_resource(struct pipe_context *pctx, struct pipe_resource *prsc)
{
   struct lima_context *ctx = lima_context(pctx);

   struct hash_entry *entry = _mesa_hash_table_search(ctx->write_jobs, prsc);
   if (!entry)
      return;

   struct lima_job *job = entry->data;
   if (job->key.zsbuf && (job->key.zsbuf->texture == prsc))
      job->resolve &= ~(PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL);

   if (job->key.cbuf && (job->key.cbuf->texture == prsc))
      job->resolve &= ~PIPE_CLEAR_COLOR0;

   _mesa_hash_table_remove_key(ctx->write_jobs, prsc);
}

static void
plb_pp_stream_delete_fn(struct hash_entry *entry)
{
   struct lima_ctx_plb_pp_stream *s = entry->data;

   lima_bo_unreference(s->bo);
   list_del(&s->lru_list);
   ralloc_free(s);
}

static void
lima_context_destroy(struct pipe_context *pctx)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_screen *screen = lima_screen(pctx->screen);

   if (ctx->jobs)
      lima_job_fini(ctx);

   for (int i = 0; i < lima_ctx_buff_num; i++)
      pipe_resource_reference(&ctx->buffer_state[i].res, NULL);

   lima_program_fini(ctx);
   lima_state_fini(ctx);
   util_unreference_framebuffer_state(&ctx->framebuffer.base);

   if (ctx->blitter)
      util_blitter_destroy(ctx->blitter);

   if (ctx->uploader)
      u_upload_destroy(ctx->uploader);

   slab_destroy_child(&ctx->transfer_pool);

   for (int i = 0; i < LIMA_CTX_PLB_MAX_NUM; i++) {
      if (ctx->plb[i])
         lima_bo_unreference(ctx->plb[i]);
      if (ctx->gp_tile_heap[i])
         lima_bo_unreference(ctx->gp_tile_heap[i]);
   }

   if (ctx->plb_gp_stream)
      lima_bo_unreference(ctx->plb_gp_stream);

   if (ctx->gp_output)
      lima_bo_unreference(ctx->gp_output);

   _mesa_hash_table_destroy(ctx->plb_pp_stream,
                            plb_pp_stream_delete_fn);

   lima_context_free_drm_ctx(screen, ctx->id);

   ralloc_free(ctx);
}

static uint32_t
plb_pp_stream_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct lima_ctx_plb_pp_stream_key));
}

static bool
plb_pp_stream_compare(const void *key1, const void *key2)
{
   return memcmp(key1, key2, sizeof(struct lima_ctx_plb_pp_stream_key)) == 0;
}

struct pipe_context *
lima_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags)
{
   struct lima_screen *screen = lima_screen(pscreen);
   struct lima_context *ctx;

   ctx = rzalloc(NULL, struct lima_context);
   if (!ctx)
      return NULL;

   ctx->id = lima_context_create_drm_ctx(screen);
   if (ctx->id < 0) {
      ralloc_free(ctx);
      return NULL;
   }

   ctx->sample_mask = (1 << LIMA_MAX_SAMPLES) - 1;

   ctx->base.screen = pscreen;
   ctx->base.destroy = lima_context_destroy;
   ctx->base.set_debug_callback = u_default_set_debug_callback;
   ctx->base.invalidate_resource = lima_invalidate_resource;

   lima_resource_context_init(ctx);
   lima_fence_context_init(ctx);
   lima_state_init(ctx);
   lima_draw_init(ctx);
   lima_program_init(ctx);
   lima_query_init(ctx);

   slab_create_child(&ctx->transfer_pool, &screen->transfer_pool);

   ctx->blitter = util_blitter_create(&ctx->base);
   if (!ctx->blitter)
      goto err_out;

   ctx->uploader = u_upload_create_default(&ctx->base);
   if (!ctx->uploader)
      goto err_out;
   ctx->base.stream_uploader = ctx->uploader;
   ctx->base.const_uploader = ctx->uploader;

   ctx->plb_size = screen->plb_max_blk * LIMA_CTX_PLB_BLK_SIZE;
   ctx->plb_gp_size = screen->plb_max_blk * 4;

   uint32_t heap_flags;
   if (screen->has_growable_heap_buffer) {
      /* growable size buffer, initially will allocate 32K (by default)
       * backup memory in kernel driver, and will allocate more when GP
       * get out of memory interrupt. Max to 16M set here.
       */
      ctx->gp_tile_heap_size = 0x1000000;
      heap_flags = LIMA_BO_FLAG_HEAP;
   } else {
      /* fix size buffer */
      ctx->gp_tile_heap_size = 0x100000;
      heap_flags = 0;
   }

   for (int i = 0; i < lima_ctx_num_plb; i++) {
      ctx->plb[i] = lima_bo_create(screen, ctx->plb_size, 0);
      if (!ctx->plb[i])
         goto err_out;
      ctx->gp_tile_heap[i] = lima_bo_create(screen, ctx->gp_tile_heap_size, heap_flags);
      if (!ctx->gp_tile_heap[i])
         goto err_out;
   }

   unsigned plb_gp_stream_size =
      align(ctx->plb_gp_size * lima_ctx_num_plb, LIMA_PAGE_SIZE);
   ctx->plb_gp_stream =
      lima_bo_create(screen, plb_gp_stream_size, 0);
   if (!ctx->plb_gp_stream)
      goto err_out;
   lima_bo_map(ctx->plb_gp_stream);

   /* plb gp stream is static for any framebuffer */
   for (int i = 0; i < lima_ctx_num_plb; i++) {
      uint32_t *plb_gp_stream = ctx->plb_gp_stream->map + i * ctx->plb_gp_size;
      for (int j = 0; j < screen->plb_max_blk; j++)
         plb_gp_stream[j] = ctx->plb[i]->va + LIMA_CTX_PLB_BLK_SIZE * j;
   }

   list_inithead(&ctx->plb_pp_stream_lru_list);
   ctx->plb_pp_stream = _mesa_hash_table_create(
      ctx, plb_pp_stream_hash, plb_pp_stream_compare);
   if (!ctx->plb_pp_stream)
      goto err_out;

   if (!lima_job_init(ctx))
      goto err_out;

   return &ctx->base;

err_out:
   lima_context_destroy(&ctx->base);
   return NULL;
}
