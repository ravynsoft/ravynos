/*
 * Copyright 2010 Christoph Bumiller
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
 */

#include "pipe/p_defines.h"
#include "util/u_framebuffer.h"
#include "util/u_upload_mgr.h"

#include "nv50/nv50_context.h"
#include "nv50/nv50_screen.h"
#include "nv50/nv50_resource.h"

static void
nv50_flush(struct pipe_context *pipe,
           struct pipe_fence_handle **fence,
           unsigned flags)
{
   struct nouveau_context *context = nouveau_context(pipe);

   if (fence)
      nouveau_fence_ref(context->fence, (struct nouveau_fence **)fence);

   PUSH_KICK(context->pushbuf);

   nouveau_context_update_frame_stats(nouveau_context(pipe));
}

static void
nv50_texture_barrier(struct pipe_context *pipe, unsigned flags)
{
   struct nouveau_pushbuf *push = nv50_context(pipe)->base.pushbuf;

   BEGIN_NV04(push, SUBC_3D(NV50_GRAPH_SERIALIZE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(TEX_CACHE_CTL), 1);
   PUSH_DATA (push, 0x20);
}

static void
nv50_memory_barrier(struct pipe_context *pipe, unsigned flags)
{
   struct nv50_context *nv50 = nv50_context(pipe);
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   int i, s;

   if (flags & PIPE_BARRIER_MAPPED_BUFFER) {
      for (i = 0; i < nv50->num_vtxbufs; ++i) {
         if (!nv50->vtxbuf[i].buffer.resource && !nv50->vtxbuf[i].is_user_buffer)
            continue;
         if (nv50->vtxbuf[i].buffer.resource->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT)
            nv50->base.vbo_dirty = true;
      }

      for (s = 0; s < NV50_MAX_3D_SHADER_STAGES && !nv50->cb_dirty; ++s) {
         uint32_t valid = nv50->constbuf_valid[s];

         while (valid && !nv50->cb_dirty) {
            const unsigned i = ffs(valid) - 1;
            struct pipe_resource *res;

            valid &= ~(1 << i);
            if (nv50->constbuf[s][i].user)
               continue;

            res = nv50->constbuf[s][i].u.buf;
            if (!res)
               continue;

            if (res->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT)
               nv50->cb_dirty = true;
         }
      }
   } else {
      BEGIN_NV04(push, SUBC_3D(NV50_GRAPH_SERIALIZE), 1);
      PUSH_DATA (push, 0);
   }

   /* If we're going to texture from a buffer/image written by a shader, we
    * must flush the texture cache.
    */
   if (flags & PIPE_BARRIER_TEXTURE) {
      BEGIN_NV04(push, NV50_3D(TEX_CACHE_CTL), 1);
      PUSH_DATA (push, 0x20);
   }

   if (flags & PIPE_BARRIER_CONSTANT_BUFFER)
      nv50->cb_dirty = true;
   if (flags & (PIPE_BARRIER_VERTEX_BUFFER | PIPE_BARRIER_INDEX_BUFFER))
      nv50->base.vbo_dirty = true;
}

static void
nv50_emit_string_marker(struct pipe_context *pipe, const char *str, int len)
{
   struct nouveau_pushbuf *push = nv50_context(pipe)->base.pushbuf;
   int string_words = len / 4;
   int data_words;

   if (len <= 0)
      return;
   string_words = MIN2(string_words, NV04_PFIFO_MAX_PACKET_LEN);
   if (string_words == NV04_PFIFO_MAX_PACKET_LEN)
      data_words = string_words;
   else
      data_words = string_words + !!(len & 3);
   BEGIN_NI04(push, SUBC_3D(NV04_GRAPH_NOP), data_words);
   if (string_words)
      PUSH_DATAp(push, str, string_words);
   if (string_words != data_words) {
      int data = 0;
      memcpy(&data, &str[string_words * 4], len & 3);
      PUSH_DATA (push, data);
   }
}

void
nv50_default_kick_notify(struct nouveau_context *context)
{
   struct nv50_context *nv50 = nv50_context(&context->pipe);

   _nouveau_fence_next(context);
   _nouveau_fence_update(context->screen, true);
   nv50->state.flushed = true;
}

static void
nv50_context_unreference_resources(struct nv50_context *nv50)
{
   unsigned s, i;

   nouveau_bufctx_del(&nv50->bufctx_3d);
   nouveau_bufctx_del(&nv50->bufctx);
   nouveau_bufctx_del(&nv50->bufctx_cp);

   util_unreference_framebuffer_state(&nv50->framebuffer);

   assert(nv50->num_vtxbufs <= PIPE_MAX_ATTRIBS);
   for (i = 0; i < nv50->num_vtxbufs; ++i)
      pipe_vertex_buffer_unreference(&nv50->vtxbuf[i]);

   for (s = 0; s < NV50_MAX_SHADER_STAGES; ++s) {
      assert(nv50->num_textures[s] <= PIPE_MAX_SAMPLERS);
      for (i = 0; i < nv50->num_textures[s]; ++i)
         pipe_sampler_view_reference(&nv50->textures[s][i], NULL);

      for (i = 0; i < NV50_MAX_PIPE_CONSTBUFS; ++i)
         if (!nv50->constbuf[s][i].user)
            pipe_resource_reference(&nv50->constbuf[s][i].u.buf, NULL);
   }

   for (i = 0; i < nv50->global_residents.size / sizeof(struct pipe_resource *);
        ++i) {
      struct pipe_resource **res = util_dynarray_element(
         &nv50->global_residents, struct pipe_resource *, i);
      pipe_resource_reference(res, NULL);
   }
   util_dynarray_fini(&nv50->global_residents);
}

static void
nv50_destroy(struct pipe_context *pipe)
{
   struct nv50_context *nv50 = nv50_context(pipe);

   simple_mtx_lock(&nv50->screen->state_lock);
   if (nv50->screen->cur_ctx == nv50) {
      nv50->screen->cur_ctx = NULL;
      /* Save off the state in case another context gets created */
      nv50->screen->save_state = nv50->state;
   }
   simple_mtx_unlock(&nv50->screen->state_lock);

   if (nv50->base.pipe.stream_uploader)
      u_upload_destroy(nv50->base.pipe.stream_uploader);

   nouveau_pushbuf_bufctx(nv50->base.pushbuf, NULL);
   PUSH_KICK(nv50->base.pushbuf);

   nv50_context_unreference_resources(nv50);

   FREE(nv50->blit);

   nouveau_fence_cleanup(&nv50->base);
   nouveau_context_destroy(&nv50->base);
}

static int
nv50_invalidate_resource_storage(struct nouveau_context *ctx,
                                 struct pipe_resource *res,
                                 int ref)
{
   struct nv50_context *nv50 = nv50_context(&ctx->pipe);
   unsigned bind = res->bind ? res->bind : PIPE_BIND_VERTEX_BUFFER;
   unsigned s, i;

   if (bind & PIPE_BIND_RENDER_TARGET) {
      assert(nv50->framebuffer.nr_cbufs <= PIPE_MAX_COLOR_BUFS);
      for (i = 0; i < nv50->framebuffer.nr_cbufs; ++i) {
         if (nv50->framebuffer.cbufs[i] &&
             nv50->framebuffer.cbufs[i]->texture == res) {
            nv50->dirty_3d |= NV50_NEW_3D_FRAMEBUFFER;
            nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_FB);
            if (!--ref)
               return ref;
         }
      }
   }
   if (bind & PIPE_BIND_DEPTH_STENCIL) {
      if (nv50->framebuffer.zsbuf &&
          nv50->framebuffer.zsbuf->texture == res) {
         nv50->dirty_3d |= NV50_NEW_3D_FRAMEBUFFER;
         nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_FB);
         if (!--ref)
            return ref;
      }
   }

   if (bind & (PIPE_BIND_VERTEX_BUFFER |
               PIPE_BIND_INDEX_BUFFER |
               PIPE_BIND_CONSTANT_BUFFER |
               PIPE_BIND_STREAM_OUTPUT |
               PIPE_BIND_SAMPLER_VIEW)) {

      assert(nv50->num_vtxbufs <= PIPE_MAX_ATTRIBS);
      for (i = 0; i < nv50->num_vtxbufs; ++i) {
         if (nv50->vtxbuf[i].buffer.resource == res) {
            nv50->dirty_3d |= NV50_NEW_3D_ARRAYS;
            nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_VERTEX);
            if (!--ref)
               return ref;
         }
      }

      for (s = 0; s < NV50_MAX_SHADER_STAGES; ++s) {
      assert(nv50->num_textures[s] <= PIPE_MAX_SAMPLERS);
      for (i = 0; i < nv50->num_textures[s]; ++i) {
         if (nv50->textures[s][i] &&
             nv50->textures[s][i]->texture == res) {
            if (unlikely(s == NV50_SHADER_STAGE_COMPUTE)) {
               nv50->dirty_cp |= NV50_NEW_CP_TEXTURES;
               nouveau_bufctx_reset(nv50->bufctx_cp, NV50_BIND_CP_TEXTURES);
            } else {
               nv50->dirty_3d |= NV50_NEW_3D_TEXTURES;
               nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_TEXTURES);
            }
            if (!--ref)
               return ref;
         }
      }
      }

      for (s = 0; s < NV50_MAX_SHADER_STAGES; ++s) {
      for (i = 0; i < NV50_MAX_PIPE_CONSTBUFS; ++i) {
         if (!(nv50->constbuf_valid[s] & (1 << i)))
            continue;
         if (!nv50->constbuf[s][i].user &&
             nv50->constbuf[s][i].u.buf == res) {
            nv50->constbuf_dirty[s] |= 1 << i;
            if (unlikely(s == NV50_SHADER_STAGE_COMPUTE)) {
               nv50->dirty_cp |= NV50_NEW_CP_CONSTBUF;
               nouveau_bufctx_reset(nv50->bufctx_cp, NV50_BIND_CP_CB(i));
            } else {
               nv50->dirty_3d |= NV50_NEW_3D_CONSTBUF;
               nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_CB(s, i));
            }
            if (!--ref)
               return ref;
         }
      }
      }
   }

   return ref;
}

static void
nv50_context_get_sample_position(struct pipe_context *, unsigned, unsigned,
                                 float *);

struct pipe_context *
nv50_create(struct pipe_screen *pscreen, void *priv, unsigned ctxflags)
{
   struct nv50_screen *screen = nv50_screen(pscreen);
   struct nv50_context *nv50;
   struct pipe_context *pipe;
   int ret;
   uint32_t flags;

   nv50 = CALLOC_STRUCT(nv50_context);
   if (!nv50)
      return NULL;
   pipe = &nv50->base.pipe;

   if (!nv50_blitctx_create(nv50))
      goto out_err;

   if (nouveau_context_init(&nv50->base, &screen->base))
      goto out_err;

   ret = nouveau_bufctx_new(nv50->base.client, 2, &nv50->bufctx);
   if (!ret)
      ret = nouveau_bufctx_new(nv50->base.client, NV50_BIND_3D_COUNT,
                               &nv50->bufctx_3d);
   if (!ret)
      ret = nouveau_bufctx_new(nv50->base.client, NV50_BIND_CP_COUNT,
                               &nv50->bufctx_cp);
   if (ret)
      goto out_err;

   nv50->base.copy_data = nv50_m2mf_copy_linear;
   nv50->base.push_data = nv50_sifc_linear_u8;
   nv50->base.push_cb   = nv50_cb_push;

   nv50->screen = screen;
   pipe->screen = pscreen;
   pipe->priv = priv;
   pipe->stream_uploader = u_upload_create_default(pipe);
   if (!pipe->stream_uploader)
      goto out_err;
   pipe->const_uploader = pipe->stream_uploader;

   pipe->destroy = nv50_destroy;

   pipe->draw_vbo = nv50_draw_vbo;
   pipe->clear = nv50_clear;
   pipe->launch_grid = nv50_launch_grid;

   pipe->flush = nv50_flush;
   pipe->texture_barrier = nv50_texture_barrier;
   pipe->memory_barrier = nv50_memory_barrier;
   pipe->get_sample_position = nv50_context_get_sample_position;
   pipe->emit_string_marker = nv50_emit_string_marker;

   simple_mtx_lock(&screen->state_lock);
   if (!screen->cur_ctx) {
      /* Restore the last context's state here, normally handled during
       * context switch
       */
      nv50->state = screen->save_state;
      screen->cur_ctx = nv50;
   }
   simple_mtx_unlock(&screen->state_lock);

   nouveau_pushbuf_bufctx(nv50->base.pushbuf, nv50->bufctx);
   nv50->base.kick_notify = nv50_default_kick_notify;
   nv50->base.pushbuf->rsvd_kick = 5;
   PUSH_SPACE(nv50->base.pushbuf, 8);

   nv50_init_query_functions(nv50);
   nv50_init_surface_functions(nv50);
   nv50_init_state_functions(nv50);
   nv50_init_resource_functions(pipe);

   nv50->base.invalidate_resource_storage = nv50_invalidate_resource_storage;

   if (screen->base.device->chipset < 0x84 ||
       debug_get_bool_option("NOUVEAU_PMPEG", false)) {
      /* PMPEG */
      nouveau_context_init_vdec(&nv50->base);
   } else if (screen->base.device->chipset < 0x98 ||
              screen->base.device->chipset == 0xa0) {
      /* VP2 */
      pipe->create_video_codec = nv84_create_decoder;
      pipe->create_video_buffer = nv84_video_buffer_create;
   } else {
      /* VP3/4 */
      pipe->create_video_codec = nv98_create_decoder;
      pipe->create_video_buffer = nv98_video_buffer_create;
   }

   flags = NOUVEAU_BO_VRAM | NOUVEAU_BO_RD;

   BCTX_REFN_bo(nv50->bufctx_3d, 3D_SCREEN, flags, screen->code);
   BCTX_REFN_bo(nv50->bufctx_3d, 3D_SCREEN, flags, screen->uniforms);
   BCTX_REFN_bo(nv50->bufctx_3d, 3D_SCREEN, flags, screen->txc);
   BCTX_REFN_bo(nv50->bufctx_3d, 3D_SCREEN, flags, screen->stack_bo);
   if (screen->compute) {
      BCTX_REFN_bo(nv50->bufctx_cp, CP_SCREEN, flags, screen->code);
      BCTX_REFN_bo(nv50->bufctx_cp, CP_SCREEN, flags, screen->uniforms);
      BCTX_REFN_bo(nv50->bufctx_cp, CP_SCREEN, flags, screen->txc);
      BCTX_REFN_bo(nv50->bufctx_cp, CP_SCREEN, flags, screen->stack_bo);
   }

   flags = NOUVEAU_BO_GART | NOUVEAU_BO_WR;

   BCTX_REFN_bo(nv50->bufctx_3d, 3D_SCREEN, flags, screen->fence.bo);
   BCTX_REFN_bo(nv50->bufctx, FENCE, flags, screen->fence.bo);
   if (screen->compute)
      BCTX_REFN_bo(nv50->bufctx_cp, CP_SCREEN, flags, screen->fence.bo);

   nv50->base.scratch.bo_size = 2 << 20;

   util_dynarray_init(&nv50->global_residents, NULL);

   // Make sure that the first TSC entry has SRGB conversion bit set, since we
   // use it as a fallback.
   if (!screen->tsc.entries[0])
      nv50_upload_tsc0(nv50);

   // And mark samplers as dirty so that the first slot would get bound to the
   // zero entry if it's not otherwise set.
   nv50->dirty_3d |= NV50_NEW_3D_SAMPLERS;

   nouveau_fence_new(&nv50->base, &nv50->base.fence);

   return pipe;

out_err:
   if (pipe->stream_uploader)
      u_upload_destroy(pipe->stream_uploader);
   if (nv50->bufctx_3d)
      nouveau_bufctx_del(&nv50->bufctx_3d);
   if (nv50->bufctx_cp)
      nouveau_bufctx_del(&nv50->bufctx_cp);
   if (nv50->bufctx)
      nouveau_bufctx_del(&nv50->bufctx);
   FREE(nv50->blit);
   FREE(nv50);
   return NULL;
}

void
nv50_bufctx_fence(struct nv50_context *nv50, struct nouveau_bufctx *bufctx, bool on_flush)
{
   struct nouveau_list *list = on_flush ? &bufctx->current : &bufctx->pending;
   struct nouveau_list *it;

   for (it = list->next; it != list; it = it->next) {
      struct nouveau_bufref *ref = (struct nouveau_bufref *)it;
      struct nv04_resource *res = ref->priv;
      if (res)
         nv50_resource_validate(nv50, res, (unsigned)ref->priv_data);
   }
}

static void
nv50_context_get_sample_position(struct pipe_context *pipe,
                                 unsigned sample_count, unsigned sample_index,
                                 float *xy)
{
   static const uint8_t ms1[1][2] = { { 0x8, 0x8 } };
   static const uint8_t ms2[2][2] = {
      { 0x4, 0x4 }, { 0xc, 0xc } }; /* surface coords (0,0), (1,0) */
   static const uint8_t ms4[4][2] = {
      { 0x6, 0x2 }, { 0xe, 0x6 },   /* (0,0), (1,0) */
      { 0x2, 0xa }, { 0xa, 0xe } }; /* (0,1), (1,1) */
   static const uint8_t ms8[8][2] = {
      { 0x1, 0x7 }, { 0x5, 0x3 },   /* (0,0), (1,0) */
      { 0x3, 0xd }, { 0x7, 0xb },   /* (0,1), (1,1) */
      { 0x9, 0x5 }, { 0xf, 0x1 },   /* (2,0), (3,0) */
      { 0xb, 0xf }, { 0xd, 0x9 } }; /* (2,1), (3,1) */

   const uint8_t (*ptr)[2];

   switch (sample_count) {
   case 0:
   case 1: ptr = ms1; break;
   case 2: ptr = ms2; break;
   case 4: ptr = ms4; break;
   case 8: ptr = ms8; break;
   default:
      assert(0);
      return; /* bad sample count -> undefined locations */
   }
   xy[0] = ptr[sample_index][0] * 0.0625f;
   xy[1] = ptr[sample_index][1] * 0.0625f;
}
