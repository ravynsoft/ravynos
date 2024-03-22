/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "etnaviv_context.h"

#include "etnaviv_blend.h"
#include "etnaviv_clear_blit.h"
#include "etnaviv_compiler.h"
#include "etnaviv_debug.h"
#include "etnaviv_emit.h"
#include "etnaviv_fence.h"
#include "etnaviv_query.h"
#include "etnaviv_query_acc.h"
#include "etnaviv_rasterizer.h"
#include "etnaviv_resource.h"
#include "etnaviv_screen.h"
#include "etnaviv_shader.h"
#include "etnaviv_state.h"
#include "etnaviv_surface.h"
#include "etnaviv_texture.h"
#include "etnaviv_transfer.h"
#include "etnaviv_translate.h"
#include "etnaviv_zsa.h"

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/hash_table.h"
#include "util/u_blitter.h"
#include "util/u_draw.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_upload_mgr.h"
#include "util/u_debug_cb.h"
#include "util/u_surface.h"
#include "util/u_transfer.h"

#include "hw/common.xml.h"

static inline void
etna_emit_nop_with_data(struct etna_cmd_stream *stream, uint32_t value)
{
   etna_cmd_stream_emit(stream, VIV_FE_NOP_HEADER_OP_NOP);
   etna_cmd_stream_emit(stream, value);
}

static void
etna_emit_string_marker(struct pipe_context *pctx, const char *string, int len)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_cmd_stream *stream = ctx->stream;
   const uint32_t *buf = (const void *)string;

   etna_cmd_stream_reserve(stream, len * 2);

   while (len >= 4) {
      etna_emit_nop_with_data(stream, *buf);
      buf++;
      len -= 4;
   }

   /* copy remainder bytes without reading past end of input string */
   if (len > 0) {
      uint32_t w = 0;
      memcpy(&w, buf, len);
      etna_emit_nop_with_data(stream, w);
   }
}

static void
etna_set_frontend_noop(struct pipe_context *pctx, bool enable)
{
   struct etna_context *ctx = etna_context(pctx);

   pctx->flush(pctx, NULL, 0);
   ctx->is_noop = enable;
}

static void
etna_context_destroy(struct pipe_context *pctx)
{
   struct etna_context *ctx = etna_context(pctx);

   if (ctx->pending_resources)
      _mesa_hash_table_destroy(ctx->pending_resources, NULL);

   if (ctx->flush_resources)
      _mesa_set_destroy(ctx->flush_resources, NULL);

   util_copy_framebuffer_state(&ctx->framebuffer_s, NULL);

   if (ctx->blitter)
      util_blitter_destroy(ctx->blitter);

   if (pctx->stream_uploader)
      u_upload_destroy(pctx->stream_uploader);

   if (ctx->stream)
      etna_cmd_stream_del(ctx->stream);

   etna_texture_fini(pctx);

   slab_destroy_child(&ctx->transfer_pool);

   if (ctx->in_fence_fd != -1)
      close(ctx->in_fence_fd);

   FREE(pctx);
}

/* Update render state where needed based on draw operation */
static void
etna_update_state_for_draw(struct etna_context *ctx, const struct pipe_draw_info *info)
{
   /* Handle primitive restart:
    * - If not an indexed draw, we don't care about the state of the primitive restart bit.
    * - Otherwise, set the bit in INDEX_STREAM_CONTROL in the index buffer state
    *   accordingly
    * - If the value of the INDEX_STREAM_CONTROL register changed due to this, or
    *   primitive restart is enabled and the restart index changed, mark the index
    *   buffer state as dirty
    */

   if (info->index_size) {
      uint32_t new_control = ctx->index_buffer.FE_INDEX_STREAM_CONTROL;

      if (info->primitive_restart)
         new_control |= VIVS_FE_INDEX_STREAM_CONTROL_PRIMITIVE_RESTART;
      else
         new_control &= ~VIVS_FE_INDEX_STREAM_CONTROL_PRIMITIVE_RESTART;

      if (ctx->index_buffer.FE_INDEX_STREAM_CONTROL != new_control ||
          (info->primitive_restart && ctx->index_buffer.FE_PRIMITIVE_RESTART_INDEX != info->restart_index)) {
         ctx->index_buffer.FE_INDEX_STREAM_CONTROL = new_control;
         ctx->index_buffer.FE_PRIMITIVE_RESTART_INDEX = info->restart_index;
         ctx->dirty |= ETNA_DIRTY_INDEX_BUFFER;
      }
   }
}

static bool
etna_get_vs(struct etna_context *ctx, struct etna_shader_key* const key)
{
   const struct etna_shader_variant *old = ctx->shader.vs;

   ctx->shader.vs = etna_shader_variant(ctx->shader.bind_vs, key, &ctx->base.debug, true);

   if (!ctx->shader.vs)
      return false;

   if (old != ctx->shader.vs)
      ctx->dirty |= ETNA_DIRTY_SHADER;

   return true;
}

static bool
etna_get_fs(struct etna_context *ctx, struct etna_shader_key* const key)
{
   const struct etna_shader_variant *old = ctx->shader.fs;

   /* update the key if we need to run nir_lower_sample_tex_compare(..). */
   if (ctx->screen->specs.halti < 2 &&
       (ctx->dirty & (ETNA_DIRTY_SAMPLERS | ETNA_DIRTY_SAMPLER_VIEWS))) {

      for (unsigned int i = 0; i < ctx->num_fragment_sampler_views; i++) {
         if (ctx->sampler[i]->compare_mode == PIPE_TEX_COMPARE_NONE)
            continue;

         key->has_sample_tex_compare = 1;
         key->num_texture_states = ctx->num_fragment_sampler_views;

         key->tex_swizzle[i].swizzle_r = ctx->sampler_view[i]->swizzle_r;
         key->tex_swizzle[i].swizzle_g = ctx->sampler_view[i]->swizzle_g;
         key->tex_swizzle[i].swizzle_b = ctx->sampler_view[i]->swizzle_b;
         key->tex_swizzle[i].swizzle_a = ctx->sampler_view[i]->swizzle_a;

         key->tex_compare_func[i] = ctx->sampler[i]->compare_func;
      }
   }

   ctx->shader.fs = etna_shader_variant(ctx->shader.bind_fs, key, &ctx->base.debug, true);

   if (!ctx->shader.fs)
      return false;

   if (old != ctx->shader.fs)
      ctx->dirty |= ETNA_DIRTY_SHADER;

   return true;
}

static void
etna_draw_vbo(struct pipe_context *pctx, const struct pipe_draw_info *info,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pctx, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

   if (!indirect && (!draws[0].count || !info->instance_count))
      return;

   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   struct pipe_framebuffer_state *pfb = &ctx->framebuffer_s;
   uint32_t draw_mode;
   unsigned i;

   if (!indirect &&
       !info->primitive_restart &&
       !u_trim_pipe_prim(info->mode, (unsigned*)&draws[0].count))
      return;

   if (ctx->vertex_elements == NULL || ctx->vertex_elements->num_elements == 0)
      return; /* Nothing to do */

   if (unlikely(ctx->rasterizer->cull_face == PIPE_FACE_FRONT_AND_BACK &&
                u_decomposed_prim(info->mode) == MESA_PRIM_TRIANGLES))
      return;

   if (!etna_render_condition_check(pctx))
      return;

   int prims = u_decomposed_prims_for_vertices(info->mode, draws[0].count);
   if (unlikely(prims <= 0)) {
      DBG("Invalid draw primitive mode=%i or no primitives to be drawn", info->mode);
      return;
   }

   draw_mode = translate_draw_mode(info->mode);
   if (draw_mode == ETNA_NO_MATCH) {
      BUG("Unsupported draw mode");
      return;
   }

   /* Upload a user index buffer. */
   unsigned index_offset = 0;
   struct pipe_resource *indexbuf = NULL;

   if (info->index_size) {
      indexbuf = info->has_user_indices ? NULL : info->index.resource;
      if (info->has_user_indices &&
          !util_upload_index_buffer(pctx, info, &draws[0], &indexbuf, &index_offset, 4)) {
         BUG("Index buffer upload failed.");
         return;
      }
      /* Add start to index offset, when rendering indexed */
      index_offset += draws[0].start * info->index_size;

      ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR.bo = etna_resource(indexbuf)->bo;
      ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR.offset = index_offset;
      ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR.flags = ETNA_RELOC_READ;
      ctx->index_buffer.FE_INDEX_STREAM_CONTROL = translate_index_size(info->index_size);

      if (!ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR.bo) {
         BUG("Unsupported or no index buffer");
         return;
      }
   } else {
      ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR.bo = 0;
      ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR.offset = 0;
      ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR.flags = 0;
      ctx->index_buffer.FE_INDEX_STREAM_CONTROL = 0;
   }
   ctx->dirty |= ETNA_DIRTY_INDEX_BUFFER;

   struct etna_shader_key key = {
      .front_ccw = ctx->rasterizer->front_ccw,
      .sprite_coord_enable = ctx->rasterizer->sprite_coord_enable,
      .sprite_coord_yinvert = !!ctx->rasterizer->sprite_coord_mode,
   };

   if (pfb->cbufs[0])
      key.frag_rb_swap = !!translate_pe_format_rb_swap(pfb->cbufs[0]->format);

   if (!etna_get_vs(ctx, &key) || !etna_get_fs(ctx, &key)) {
      BUG("compiled shaders are not okay");
      return;
   }

   /* Update any derived state */
   if (!etna_state_update(ctx))
      return;

   /*
    * Figure out the buffers/features we need:
    */
   if (ctx->dirty & ETNA_DIRTY_ZSA) {
      if (etna_depth_enabled(ctx))
         resource_written(ctx, pfb->zsbuf->texture);

      if (etna_stencil_enabled(ctx))
         resource_written(ctx, pfb->zsbuf->texture);
   }

   if (ctx->dirty & ETNA_DIRTY_FRAMEBUFFER) {
      for (i = 0; i < pfb->nr_cbufs; i++) {
         struct pipe_resource *surf;

         if (!pfb->cbufs[i])
            continue;

         surf = pfb->cbufs[i]->texture;
         resource_written(ctx, surf);
      }
   }

   if (ctx->dirty & ETNA_DIRTY_SHADER) {
      /* Mark constant buffers as being read */
      u_foreach_bit(i, ctx->constant_buffer[PIPE_SHADER_VERTEX].enabled_mask)
         resource_read(ctx, ctx->constant_buffer[PIPE_SHADER_VERTEX].cb[i].buffer);

      u_foreach_bit(i, ctx->constant_buffer[PIPE_SHADER_FRAGMENT].enabled_mask)
         resource_read(ctx, ctx->constant_buffer[PIPE_SHADER_FRAGMENT].cb[i].buffer);
   }

   if (ctx->dirty & ETNA_DIRTY_VERTEX_BUFFERS) {
      /* Mark VBOs as being read */
      u_foreach_bit(i, ctx->vertex_buffer.enabled_mask) {
         assert(!ctx->vertex_buffer.vb[i].is_user_buffer);
         resource_read(ctx, ctx->vertex_buffer.vb[i].buffer.resource);
      }
   }

   if (ctx->dirty & ETNA_DIRTY_INDEX_BUFFER) {
      /* Mark index buffer as being read */
      resource_read(ctx, indexbuf);
   }

   /* Mark textures as being read */
   for (i = 0; i < PIPE_MAX_SAMPLERS; i++) {
      if (ctx->sampler_view[i]) {
         if (ctx->dirty & ETNA_DIRTY_SAMPLER_VIEWS)
             resource_read(ctx, ctx->sampler_view[i]->texture);

         /* if texture was modified since the last update,
          * we need to clear the texture cache and possibly
          * resolve/update ts
          */
         etna_update_sampler_source(ctx->sampler_view[i], i);
      }
   }

   ctx->stats.prims_generated += u_reduced_prims_for_vertices(info->mode, draws[0].count);
   ctx->stats.draw_calls++;

   /* Update state for this draw operation */
   etna_update_state_for_draw(ctx, info);

   /* First, sync state, then emit DRAW_PRIMITIVES or DRAW_INDEXED_PRIMITIVES */
   etna_emit_state(ctx);

   if (!VIV_FEATURE(screen, chipMinorFeatures6, NEW_GPIPE)) {
      switch (draw_mode) {
      case PRIMITIVE_TYPE_LINE_LOOP:
      case PRIMITIVE_TYPE_LINE_STRIP:
      case PRIMITIVE_TYPE_TRIANGLE_STRIP:
      case PRIMITIVE_TYPE_TRIANGLE_FAN:
         etna_set_state(ctx->stream, VIVS_GL_VERTEX_ELEMENT_CONFIG,
                        VIVS_GL_VERTEX_ELEMENT_CONFIG_UNK0 |
                        VIVS_GL_VERTEX_ELEMENT_CONFIG_REUSE);
         break;
      default:
         etna_set_state(ctx->stream, VIVS_GL_VERTEX_ELEMENT_CONFIG,
                        VIVS_GL_VERTEX_ELEMENT_CONFIG_UNK0);
         break;
      }
   }

   if (screen->specs.halti >= 2) {
      /* On HALTI2+ (GC3000 and higher) only use instanced drawing commands, as the blob does */
      etna_draw_instanced(ctx->stream, info->index_size, draw_mode, info->instance_count,
         draws[0].count, info->index_size ? draws->index_bias : draws[0].start);
   } else {
      if (info->index_size)
         etna_draw_indexed_primitives(ctx->stream, draw_mode, 0, prims, draws->index_bias);
      else
         etna_draw_primitives(ctx->stream, draw_mode, draws[0].start, prims);
   }

   if (DBG_ENABLED(ETNA_DBG_DRAW_STALL)) {
      /* Stall the FE after every draw operation.  This allows better
       * debug of GPU hang conditions, as the FE will indicate which
       * draw op has caused the hang. */
      etna_stall(ctx->stream, SYNC_RECIPIENT_FE, SYNC_RECIPIENT_PE);
   }

   if (DBG_ENABLED(ETNA_DBG_FLUSH_ALL))
      pctx->flush(pctx, NULL, 0);

   if (ctx->framebuffer_s.cbufs[0])
      etna_resource_level_mark_changed(etna_surface(ctx->framebuffer_s.cbufs[0])->level);
   if (ctx->framebuffer_s.zsbuf)
      etna_resource_level_mark_changed(etna_surface(ctx->framebuffer_s.zsbuf)->level);
   if (info->index_size && indexbuf != info->index.resource)
      pipe_resource_reference(&indexbuf, NULL);
}

static void
etna_reset_gpu_state(struct etna_context *ctx)
{
   struct etna_cmd_stream *stream = ctx->stream;
   struct etna_screen *screen = ctx->screen;
   uint32_t dummy_attribs[VIVS_NFE_GENERIC_ATTRIB__LEN] = { 0 };

   etna_set_state(stream, VIVS_GL_API_MODE, VIVS_GL_API_MODE_OPENGL);
   etna_set_state(stream, VIVS_PA_W_CLIP_LIMIT, 0x34000001);
   etna_set_state(stream, VIVS_PA_FLAGS, 0x00000000); /* blob sets ZCONVERT_BYPASS on GC3000+, this messes up z for us */
   etna_set_state(stream, VIVS_PA_VIEWPORT_UNK00A80, 0x38a01404);
   etna_set_state(stream, VIVS_PA_VIEWPORT_UNK00A84, fui(8192.0));
   etna_set_state(stream, VIVS_PA_ZFARCLIPPING, 0x00000000);
   etna_set_state(stream, VIVS_RA_HDEPTH_CONTROL, 0x00007000);
   etna_set_state(stream, VIVS_PS_CONTROL_EXT, 0x00000000);

   /* There is no HALTI0 specific state */
   if (screen->specs.halti >= 1) { /* Only on HALTI1+ */
      etna_set_state(stream, VIVS_VS_HALTI1_UNK00884, 0x00000808);
   }
   if (screen->specs.halti >= 2) { /* Only on HALTI2+ */
      etna_set_state(stream, VIVS_RA_UNK00E0C, 0x00000000);
   }
   if (screen->specs.halti >= 3) { /* Only on HALTI3+ */
      etna_set_state(stream, VIVS_PS_HALTI3_UNK0103C, 0x76543210);
   }
   if (screen->specs.halti >= 4) { /* Only on HALTI4+ */
      etna_set_state(stream, VIVS_PS_MSAA_CONFIG, 0x6fffffff & 0xf70fffff & 0xfff6ffff &
                                                  0xffff6fff & 0xfffff6ff & 0xffffff7f);
      etna_set_state(stream, VIVS_PE_HALTI4_UNK014C0, 0x00000000);
   }
   if (screen->specs.halti >= 5) { /* Only on HALTI5+ */
      etna_set_state(stream, VIVS_NTE_DESCRIPTOR_UNK14C40, 0x00000001);
      etna_set_state(stream, VIVS_FE_HALTI5_UNK007D8, 0x00000002);
      etna_set_state(stream, VIVS_PS_SAMPLER_BASE, 0x00000000);
      etna_set_state(stream, VIVS_VS_SAMPLER_BASE, 0x00000020);
      etna_set_state(stream, VIVS_SH_CONFIG, VIVS_SH_CONFIG_RTNE_ROUNDING);
   } else { /* Only on pre-HALTI5 */
      etna_set_state(stream, VIVS_GL_UNK03838, 0x00000000);
      etna_set_state(stream, VIVS_GL_UNK03854, 0x00000000);
   }

   if (VIV_FEATURE(screen, chipMinorFeatures4, BUG_FIXES18))
      etna_set_state(stream, VIVS_GL_BUG_FIXES, 0x6);

   if (!screen->specs.use_blt) {
      /* Enable SINGLE_BUFFER for resolve, if supported */
      etna_set_state(stream, VIVS_RS_SINGLE_BUFFER, COND(screen->specs.single_buffer, VIVS_RS_SINGLE_BUFFER_ENABLE));
   }

   if (screen->specs.halti >= 5) {
      /* TXDESC cache flush - do this once at the beginning, as texture
       * descriptors are only written by the CPU once, then patched by the kernel
       * before command stream submission. It does not need flushing if the
       * referenced image data changes.
       */
      etna_set_state(stream, VIVS_NTE_DESCRIPTOR_FLUSH, 0);
      etna_set_state(stream, VIVS_GL_FLUSH_CACHE,
            VIVS_GL_FLUSH_CACHE_DESCRIPTOR_UNK12 |
            VIVS_GL_FLUSH_CACHE_DESCRIPTOR_UNK13);

      /* Icache invalidate (should do this on shader change?) */
      etna_set_state(stream, VIVS_VS_ICACHE_INVALIDATE,
            VIVS_VS_ICACHE_INVALIDATE_UNK0 | VIVS_VS_ICACHE_INVALIDATE_UNK1 |
            VIVS_VS_ICACHE_INVALIDATE_UNK2 | VIVS_VS_ICACHE_INVALIDATE_UNK3 |
            VIVS_VS_ICACHE_INVALIDATE_UNK4);
   }

   /* It seems that some GPUs (at least some GC400 have shown this behavior)
    * come out of reset with random vertex attributes enabled and also don't
    * disable them on the write to the first config register as normal. Enabling
    * all attributes seems to provide the GPU with the required edge to actually
    * disable the unused attributes on the next draw.
    */
   if (screen->specs.halti >= 5) {
      etna_set_state_multi(stream, VIVS_NFE_GENERIC_ATTRIB_CONFIG0(0),
                           VIVS_NFE_GENERIC_ATTRIB__LEN, dummy_attribs);
   } else {
      etna_set_state_multi(stream, VIVS_FE_VERTEX_ELEMENT_CONFIG(0),
                           screen->specs.halti >= 0 ? 16 : 12, dummy_attribs);
   }

   etna_cmd_stream_mark_end_of_context_init(stream);

   ctx->dirty = ~0L;
   ctx->dirty_sampler_views = ~0L;
   ctx->prev_active_samplers = ~0L;
}

void
etna_flush(struct pipe_context *pctx, struct pipe_fence_handle **fence,
           enum pipe_flush_flags flags, bool internal)
{
   struct etna_context *ctx = etna_context(pctx);
   int out_fence_fd = -1;

   list_for_each_entry(struct etna_acc_query, aq, &ctx->active_acc_queries, node)
      etna_acc_query_suspend(aq, ctx);

   if (!internal) {
      /* flush all resources that need an implicit flush */
      set_foreach(ctx->flush_resources, entry) {
         struct pipe_resource *prsc = (struct pipe_resource *)entry->key;

         pctx->flush_resource(pctx, prsc);
         pipe_resource_reference(&prsc, NULL);
      }
      _mesa_set_clear(ctx->flush_resources, NULL);
   }

   etna_cmd_stream_flush(ctx->stream, ctx->in_fence_fd,
                          (flags & PIPE_FLUSH_FENCE_FD) ? &out_fence_fd : NULL,
                          ctx->is_noop);

   list_for_each_entry(struct etna_acc_query, aq, &ctx->active_acc_queries, node)
      etna_acc_query_resume(aq, ctx);

   if (fence)
      *fence = etna_fence_create(pctx, out_fence_fd);

   _mesa_hash_table_clear(ctx->pending_resources, NULL);

   etna_reset_gpu_state(ctx);
}

static void
etna_context_flush(struct pipe_context *pctx, struct pipe_fence_handle **fence,
                   enum pipe_flush_flags flags)
{
   etna_flush(pctx, fence, flags, false);
}

static void
etna_context_force_flush(struct etna_cmd_stream *stream, void *priv)
{
   struct pipe_context *pctx = priv;

   etna_flush(pctx, NULL, 0, true);

   /* update derived states as the context is now fully dirty */
   etna_state_update(etna_context(pctx));
}

void
etna_context_add_flush_resource(struct etna_context *ctx,
                                struct pipe_resource *rsc)
{
   bool found;

   _mesa_set_search_or_add(ctx->flush_resources, rsc, &found);

   if (!found)
      pipe_reference(NULL, &rsc->reference);
}

static void
etna_set_debug_callback(struct pipe_context *pctx,
                        const struct util_debug_callback *cb)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;

   util_queue_finish(&screen->shader_compiler_queue);
   u_default_set_debug_callback(pctx, cb);
}

struct pipe_context *
etna_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags)
{
   struct etna_context *ctx = CALLOC_STRUCT(etna_context);
   struct etna_screen *screen;
   struct pipe_context *pctx;

   if (ctx == NULL)
      return NULL;

   pctx = &ctx->base;
   pctx->priv = ctx;
   pctx->screen = pscreen;
   pctx->stream_uploader = u_upload_create_default(pctx);
   if (!pctx->stream_uploader)
      goto fail;
   pctx->const_uploader = pctx->stream_uploader;

   screen = etna_screen(pscreen);
   ctx->stream = etna_cmd_stream_new(screen->pipe, 0x2000,
                                     &etna_context_force_flush, pctx);
   if (ctx->stream == NULL)
      goto fail;

   ctx->pending_resources = _mesa_pointer_hash_table_create(NULL);
   if (!ctx->pending_resources)
      goto fail;

   ctx->flush_resources = _mesa_set_create(NULL, _mesa_hash_pointer,
                                           _mesa_key_pointer_equal);
   if (!ctx->flush_resources)
      goto fail;

   /* context ctxate setup */
   ctx->screen = screen;
   /* need some sane default in case gallium frontends don't set some state: */
   ctx->sample_mask = 0xffff;

   /*  Set sensible defaults for state */
   etna_reset_gpu_state(ctx);

   ctx->in_fence_fd = -1;

   pctx->destroy = etna_context_destroy;
   pctx->draw_vbo = etna_draw_vbo;
   pctx->flush = etna_context_flush;
   pctx->set_debug_callback = etna_set_debug_callback;
   pctx->create_fence_fd = etna_create_fence_fd;
   pctx->fence_server_sync = etna_fence_server_sync;
   pctx->emit_string_marker = etna_emit_string_marker;
   pctx->set_frontend_noop = etna_set_frontend_noop;
   pctx->clear_buffer = u_default_clear_buffer;
   pctx->clear_texture = u_default_clear_texture;

   /* creation of compile states */
   pctx->create_blend_state = etna_blend_state_create;
   pctx->create_rasterizer_state = etna_rasterizer_state_create;
   pctx->create_depth_stencil_alpha_state = etna_zsa_state_create;

   etna_clear_blit_init(pctx);
   etna_query_context_init(pctx);
   etna_state_init(pctx);
   etna_surface_init(pctx);
   etna_shader_init(pctx);
   etna_texture_init(pctx);
   etna_transfer_init(pctx);

   ctx->blitter = util_blitter_create(pctx);
   if (!ctx->blitter)
      goto fail;

   slab_create_child(&ctx->transfer_pool, &screen->transfer_pool);
   list_inithead(&ctx->active_acc_queries);

   return pctx;

fail:
   pctx->destroy(pctx);

   return NULL;
}

bool
etna_render_condition_check(struct pipe_context *pctx)
{
   struct etna_context *ctx = etna_context(pctx);

   if (!ctx->cond_query)
      return true;

   perf_debug_ctx(ctx, "Implementing conditional rendering on the CPU");

   union pipe_query_result res = { 0 };
   bool wait =
      ctx->cond_mode != PIPE_RENDER_COND_NO_WAIT &&
      ctx->cond_mode != PIPE_RENDER_COND_BY_REGION_NO_WAIT;

   if (pctx->get_query_result(pctx, ctx->cond_query, wait, &res))
      return (bool)res.u64 != ctx->cond_cond;

   return true;
}
