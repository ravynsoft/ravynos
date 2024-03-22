/*
 * Copyright 2012 Red Hat Inc.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 *
 */

#include "draw/draw_context.h"
#include "util/u_upload_mgr.h"

#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"

#include "nouveau_fence.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_transfer.h"
#include "nv30/nv30_state.h"
#include "nv30/nv30_winsys.h"

static void
nv30_context_kick_notify(struct nouveau_pushbuf *push)
{
   struct nouveau_pushbuf_priv *p = push->user_priv;
   struct nouveau_screen *screen = p->screen;

   _nouveau_fence_next(p->context);
   _nouveau_fence_update(screen, true);

   if (push->bufctx) {
      struct nouveau_bufref *bref;
      LIST_FOR_EACH_ENTRY(bref, &push->bufctx->current, thead) {
         struct nv04_resource *res = bref->priv;
         if (res && res->mm) {
            _nouveau_fence_ref(p->context->fence, &res->fence);

            if (bref->flags & NOUVEAU_BO_RD)
               res->status |= NOUVEAU_BUFFER_STATUS_GPU_READING;

            if (bref->flags & NOUVEAU_BO_WR) {
               _nouveau_fence_ref(p->context->fence, &res->fence_wr);
               res->status |= NOUVEAU_BUFFER_STATUS_GPU_WRITING |
                  NOUVEAU_BUFFER_STATUS_DIRTY;
            }
         }
      }
   }
}

static void
nv30_context_flush(struct pipe_context *pipe, struct pipe_fence_handle **fence,
                   unsigned flags)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   if (fence)
      nouveau_fence_ref(nv30->base.fence,
                        (struct nouveau_fence **)fence);

   PUSH_KICK(push);

   nouveau_context_update_frame_stats(&nv30->base);
}

static int
nv30_invalidate_resource_storage(struct nouveau_context *nv,
                                 struct pipe_resource *res,
                                 int ref)
{
   struct nv30_context *nv30 = nv30_context(&nv->pipe);
   unsigned i;

   if (res->bind & PIPE_BIND_RENDER_TARGET) {
      for (i = 0; i < nv30->framebuffer.nr_cbufs; ++i) {
         if (nv30->framebuffer.cbufs[i] &&
             nv30->framebuffer.cbufs[i]->texture == res) {
            nv30->dirty |= NV30_NEW_FRAMEBUFFER;
            nouveau_bufctx_reset(nv30->bufctx, BUFCTX_FB);
            if (!--ref)
               return ref;
         }
      }
   }
   if (res->bind & PIPE_BIND_DEPTH_STENCIL) {
      if (nv30->framebuffer.zsbuf &&
          nv30->framebuffer.zsbuf->texture == res) {
            nv30->dirty |= NV30_NEW_FRAMEBUFFER;
            nouveau_bufctx_reset(nv30->bufctx, BUFCTX_FB);
            if (!--ref)
               return ref;
      }
   }

   if (res->bind & PIPE_BIND_VERTEX_BUFFER) {
      for (i = 0; i < nv30->num_vtxbufs; ++i) {
         if (nv30->vtxbuf[i].buffer.resource == res) {
            nv30->dirty |= NV30_NEW_ARRAYS;
            nouveau_bufctx_reset(nv30->bufctx, BUFCTX_VTXBUF);
            if (!--ref)
               return ref;
         }
      }
   }

   if (res->bind & PIPE_BIND_SAMPLER_VIEW) {
      for (i = 0; i < nv30->fragprog.num_textures; ++i) {
         if (nv30->fragprog.textures[i] &&
             nv30->fragprog.textures[i]->texture == res) {
            nv30->dirty |= NV30_NEW_FRAGTEX;
            nouveau_bufctx_reset(nv30->bufctx, BUFCTX_FRAGTEX(i));
            if (!--ref)
               return ref;
         }
      }
      for (i = 0; i < nv30->vertprog.num_textures; ++i) {
         if (nv30->vertprog.textures[i] &&
             nv30->vertprog.textures[i]->texture == res) {
            nv30->dirty |= NV30_NEW_VERTTEX;
            nouveau_bufctx_reset(nv30->bufctx, BUFCTX_VERTTEX(i));
            if (!--ref)
               return ref;
         }
      }
   }

   return ref;
}

static void
nv30_context_destroy(struct pipe_context *pipe)
{
   struct nv30_context *nv30 = nv30_context(pipe);

   if (nv30->blitter)
      util_blitter_destroy(nv30->blitter);

   if (nv30->draw)
      draw_destroy(nv30->draw);

   if (nv30->base.pipe.stream_uploader)
      u_upload_destroy(nv30->base.pipe.stream_uploader);

   if (nv30->blit_vp)
      nouveau_heap_free(&nv30->blit_vp);

   if (nv30->blit_fp)
      pipe_resource_reference(&nv30->blit_fp, NULL);

   nouveau_bufctx_del(&nv30->bufctx);

   if (nv30->screen->cur_ctx == nv30)
      nv30->screen->cur_ctx = NULL;

   nouveau_fence_cleanup(&nv30->base);
   nouveau_context_destroy(&nv30->base);
}

#define FAIL_CONTEXT_INIT(str, err)                   \
   do {                                               \
      NOUVEAU_ERR(str, err);                          \
      nv30_context_destroy(pipe);                     \
      return NULL;                                    \
   } while(0)

struct pipe_context *
nv30_context_create(struct pipe_screen *pscreen, void *priv, unsigned ctxflags)
{
   struct nv30_screen *screen = nv30_screen(pscreen);
   struct nv30_context *nv30 = CALLOC_STRUCT(nv30_context);
   struct pipe_context *pipe;
   int ret;

   if (!nv30)
      return NULL;

   nv30->screen = screen;
   nv30->base.copy_data = nv30_transfer_copy_data;

   pipe = &nv30->base.pipe;
   pipe->screen = pscreen;
   pipe->priv = priv;
   pipe->destroy = nv30_context_destroy;
   pipe->flush = nv30_context_flush;

   if (nouveau_context_init(&nv30->base, &screen->base)) {
      nv30_context_destroy(pipe);
      return NULL;
   }
   nv30->base.pushbuf->kick_notify = nv30_context_kick_notify;

   nv30->base.pipe.stream_uploader = u_upload_create_default(&nv30->base.pipe);
   if (!nv30->base.pipe.stream_uploader) {
      nv30_context_destroy(pipe);
      return NULL;
   }
   nv30->base.pipe.const_uploader = nv30->base.pipe.stream_uploader;

   nv30->base.invalidate_resource_storage = nv30_invalidate_resource_storage;

   ret = nouveau_bufctx_new(nv30->base.client, 64, &nv30->bufctx);
   if (ret) {
      nv30_context_destroy(pipe);
      return NULL;
   }

   /*XXX: make configurable with performance vs quality, these defaults
    *     match the binary driver's defaults
    */
   if (screen->eng3d->oclass < NV40_3D_CLASS)
      nv30->config.filter = 0x00000004;
   else
      nv30->config.filter = 0x00002dc4;

   nv30->config.aniso = NV40_3D_TEX_WRAP_ANISO_MIP_FILTER_OPTIMIZATION_OFF;

   if (debug_get_bool_option("NV30_SWTNL", false))
      nv30->draw_flags |= NV30_NEW_SWTNL;

   nv30->sample_mask = 0xffff;
   nv30_vbo_init(pipe);
   nv30_query_init(pipe);
   nv30_state_init(pipe);
   nv30_resource_init(pipe);
   nv30_clear_init(pipe);
   nv30_fragprog_init(pipe);
   nv30_vertprog_init(pipe);
   nv30_texture_init(pipe);
   nv30_fragtex_init(pipe);
   nv40_verttex_init(pipe);
   nv30_draw_init(pipe);

   nv30->blitter = util_blitter_create(pipe);
   if (!nv30->blitter) {
      nv30_context_destroy(pipe);
      return NULL;
   }

   nouveau_context_init_vdec(&nv30->base);
   nouveau_fence_new(&nv30->base, &nv30->base.fence);

   return pipe;
}
