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
  * @file
  *
  * Wrap the cso cache & hash mechanisms in a simplified
  * pipe-driver-specific interface.
  *
  * @author Zack Rusin <zackr@vmware.com>
  * @author Keith Whitwell <keithw@vmware.com>
  */

#include "pipe/p_state.h"
#include "util/u_draw.h"
#include "util/u_framebuffer.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_vbuf.h"

#include "cso_cache/cso_context.h"
#include "cso_cache/cso_cache.h"
#include "cso_cache/cso_hash.h"
#include "cso_context.h"
#include "driver_trace/tr_dump.h"
#include "util/u_threaded_context.h"

/**
 * Per-shader sampler information.
 */
struct sampler_info
{
   struct cso_sampler *cso_samplers[PIPE_MAX_SAMPLERS];
   void *samplers[PIPE_MAX_SAMPLERS];
};



struct cso_context_priv {
   struct cso_context base;

   struct u_vbuf *vbuf;
   struct u_vbuf *vbuf_current;
   bool always_use_vbuf;
   bool sampler_format;

   bool has_geometry_shader;
   bool has_tessellation;
   bool has_compute_shader;
   bool has_task_mesh_shader;
   bool has_streamout;

   uint32_t max_fs_samplerviews : 16;

   unsigned saved_state;  /**< bitmask of CSO_BIT_x flags */
   unsigned saved_compute_state;  /**< bitmask of CSO_BIT_COMPUTE_x flags */

   struct sampler_info fragment_samplers_saved;
   struct sampler_info compute_samplers_saved;
   struct sampler_info samplers[PIPE_SHADER_MESH_TYPES];

   /* Temporary number until cso_single_sampler_done is called.
    * It tracks the highest sampler seen in cso_single_sampler.
    */
   int max_sampler_seen;

   unsigned nr_so_targets;
   struct pipe_stream_output_target *so_targets[PIPE_MAX_SO_BUFFERS];

   unsigned nr_so_targets_saved;
   struct pipe_stream_output_target *so_targets_saved[PIPE_MAX_SO_BUFFERS];

   /** Current and saved state.
    * The saved state is used as a 1-deep stack.
    */
   void *blend, *blend_saved;
   void *depth_stencil, *depth_stencil_saved;
   void *rasterizer, *rasterizer_saved;
   void *fragment_shader, *fragment_shader_saved;
   void *vertex_shader, *vertex_shader_saved;
   void *geometry_shader, *geometry_shader_saved;
   void *tessctrl_shader, *tessctrl_shader_saved;
   void *tesseval_shader, *tesseval_shader_saved;
   void *compute_shader, *compute_shader_saved;
   void *velements, *velements_saved;
   struct pipe_query *render_condition, *render_condition_saved;
   enum pipe_render_cond_flag render_condition_mode, render_condition_mode_saved;
   bool render_condition_cond, render_condition_cond_saved;
   bool flatshade_first, flatshade_first_saved;

   struct pipe_framebuffer_state fb, fb_saved;
   struct pipe_viewport_state vp, vp_saved;
   unsigned sample_mask, sample_mask_saved;
   unsigned min_samples, min_samples_saved;
   struct pipe_stencil_ref stencil_ref, stencil_ref_saved;

   /* This should be last to keep all of the above together in memory. */
   struct cso_cache cache;
};


static inline bool
delete_cso(struct cso_context_priv *ctx,
           void *state, enum cso_cache_type type)
{
   switch (type) {
   case CSO_BLEND:
      if (ctx->blend == ((struct cso_blend*)state)->data ||
          ctx->blend_saved == ((struct cso_blend*)state)->data)
         return false;
      break;
   case CSO_DEPTH_STENCIL_ALPHA:
      if (ctx->depth_stencil == ((struct cso_depth_stencil_alpha*)state)->data ||
          ctx->depth_stencil_saved == ((struct cso_depth_stencil_alpha*)state)->data)
         return false;
      break;
   case CSO_RASTERIZER:
      if (ctx->rasterizer == ((struct cso_rasterizer*)state)->data ||
          ctx->rasterizer_saved == ((struct cso_rasterizer*)state)->data)
         return false;
      break;
   case CSO_VELEMENTS:
      if (ctx->velements == ((struct cso_velements*)state)->data ||
          ctx->velements_saved == ((struct cso_velements*)state)->data)
         return false;
      break;
   case CSO_SAMPLER:
      /* nothing to do for samplers */
      break;
   default:
      assert(0);
   }

   cso_delete_state(ctx->base.pipe, state, type);
   return true;
}


static inline void
sanitize_hash(struct cso_hash *hash, enum cso_cache_type type,
              int max_size, void *user_data)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)user_data;
   /* if we're approach the maximum size, remove fourth of the entries
    * otherwise every subsequent call will go through the same */
   const int hash_size = cso_hash_size(hash);
   const int max_entries = (max_size > hash_size) ? max_size : hash_size;
   int to_remove =  (max_size < max_entries) * max_entries/4;
   struct cso_sampler **samplers_to_restore = NULL;
   unsigned to_restore = 0;

   if (hash_size > max_size)
      to_remove += hash_size - max_size;

   if (to_remove == 0)
      return;

   if (type == CSO_SAMPLER) {
      samplers_to_restore = MALLOC((PIPE_SHADER_MESH_TYPES + 2) * PIPE_MAX_SAMPLERS *
                                   sizeof(*samplers_to_restore));

      /* Temporarily remove currently bound sampler states from the hash
       * table, to prevent them from being deleted
       */
      for (int i = 0; i < PIPE_SHADER_MESH_TYPES; i++) {
         for (int j = 0; j < PIPE_MAX_SAMPLERS; j++) {
            struct cso_sampler *sampler = ctx->samplers[i].cso_samplers[j];

            if (sampler && cso_hash_take(hash, sampler->hash_key))
               samplers_to_restore[to_restore++] = sampler;
         }
      }
      for (int j = 0; j < PIPE_MAX_SAMPLERS; j++) {
         struct cso_sampler *sampler = ctx->fragment_samplers_saved.cso_samplers[j];

         if (sampler && cso_hash_take(hash, sampler->hash_key))
            samplers_to_restore[to_restore++] = sampler;
      }
      for (int j = 0; j < PIPE_MAX_SAMPLERS; j++) {
         struct cso_sampler *sampler = ctx->compute_samplers_saved.cso_samplers[j];

         if (sampler && cso_hash_take(hash, sampler->hash_key))
            samplers_to_restore[to_restore++] = sampler;
      }
   }

   struct cso_hash_iter iter = cso_hash_first_node(hash);
   while (to_remove) {
      /*remove elements until we're good */
      /*fixme: currently we pick the nodes to remove at random*/
      void *cso = cso_hash_iter_data(iter);

      if (!cso)
         break;

      if (delete_cso(ctx, cso, type)) {
         iter = cso_hash_erase(hash, iter);
         --to_remove;
      } else {
         iter = cso_hash_iter_next(iter);
      }
   }

   if (type == CSO_SAMPLER) {
      /* Put currently bound sampler states back into the hash table */
      while (to_restore--) {
         struct cso_sampler *sampler = samplers_to_restore[to_restore];

         cso_hash_insert(hash, sampler->hash_key, sampler);
      }

      FREE(samplers_to_restore);
   }
}


static void
cso_init_vbuf(struct cso_context_priv *cso, unsigned flags)
{
   struct u_vbuf_caps caps;
   bool uses_user_vertex_buffers = !(flags & CSO_NO_USER_VERTEX_BUFFERS);
   bool needs64b = !(flags & CSO_NO_64B_VERTEX_BUFFERS);

   u_vbuf_get_caps(cso->base.pipe->screen, &caps, needs64b);

   /* Enable u_vbuf if needed. */
   if (caps.fallback_always ||
       (uses_user_vertex_buffers &&
        caps.fallback_only_for_user_vbuffers)) {
      assert(!cso->base.pipe->vbuf);
      cso->vbuf = u_vbuf_create(cso->base.pipe, &caps);
      cso->base.pipe->vbuf = cso->vbuf;
      cso->always_use_vbuf = caps.fallback_always;
      cso->vbuf_current = cso->base.pipe->vbuf =
         caps.fallback_always ? cso->vbuf : NULL;
   }
}

static void
cso_draw_vbo_default(struct pipe_context *pipe,
                     const struct pipe_draw_info *info,
                     unsigned drawid_offset,
                     const struct pipe_draw_indirect_info *indirect,
                     const struct pipe_draw_start_count_bias *draws,
                     unsigned num_draws)
{
   if (pipe->vbuf)
      u_vbuf_draw_vbo(pipe, info, drawid_offset, indirect, draws, num_draws);
   else
      pipe->draw_vbo(pipe, info, drawid_offset, indirect, draws, num_draws);
}

struct cso_context *
cso_create_context(struct pipe_context *pipe, unsigned flags)
{
   struct cso_context_priv *ctx = CALLOC_STRUCT(cso_context_priv);
   if (!ctx)
      return NULL;

   cso_cache_init(&ctx->cache, pipe);
   cso_cache_set_sanitize_callback(&ctx->cache, sanitize_hash, ctx);

   ctx->base.pipe = pipe;
   ctx->sample_mask = ~0;

   if (!(flags & CSO_NO_VBUF))
      cso_init_vbuf(ctx, flags);

   /* Only drivers using u_threaded_context benefit from the direct call.
    * This is because drivers can change draw_vbo, but u_threaded_context
    * never changes it.
    */
   if (pipe->draw_vbo == tc_draw_vbo) {
      if (ctx->vbuf_current)
         ctx->base.draw_vbo = u_vbuf_draw_vbo;
      else
         ctx->base.draw_vbo = pipe->draw_vbo;
   } else if (ctx->always_use_vbuf) {
      ctx->base.draw_vbo = u_vbuf_draw_vbo;
   } else {
      ctx->base.draw_vbo = cso_draw_vbo_default;
   }

   /* Enable for testing: */
   if (0) cso_set_maximum_cache_size(&ctx->cache, 4);

   if (pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_GEOMETRY,
                                PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0) {
      ctx->has_geometry_shader = true;
   }
   if (pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_TESS_CTRL,
                                PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0) {
      ctx->has_tessellation = true;
   }
   if (pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_COMPUTE,
                                      PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0) {
      int supported_irs =
         pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_COMPUTE,
                                        PIPE_SHADER_CAP_SUPPORTED_IRS);
      if (supported_irs & ((1 << PIPE_SHADER_IR_TGSI) |
                           (1 << PIPE_SHADER_IR_NIR))) {
         ctx->has_compute_shader = true;
      }
   }
   if (pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_MESH,
                                PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0) {
      ctx->has_task_mesh_shader = true;
   }
   if (pipe->screen->get_param(pipe->screen,
                               PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS) != 0) {
      ctx->has_streamout = true;
   }

   if (pipe->screen->get_param(pipe->screen,
                               PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK) &
       PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_FREEDRENO)
      ctx->sampler_format = true;

   ctx->max_fs_samplerviews =
      pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_FRAGMENT,
                                     PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS);

   ctx->max_sampler_seen = -1;
   return &ctx->base;
}


void
cso_unbind_context(struct cso_context *cso)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   unsigned i;

   bool dumping = trace_dumping_enabled_locked();
   if (dumping)
      trace_dumping_stop_locked();
   if (ctx->base.pipe) {
      ctx->base.pipe->bind_blend_state(ctx->base.pipe, NULL);
      ctx->base.pipe->bind_rasterizer_state(ctx->base.pipe, NULL);

      {
         static struct pipe_sampler_view *views[PIPE_MAX_SHADER_SAMPLER_VIEWS] = { NULL };
         static struct pipe_shader_buffer ssbos[PIPE_MAX_SHADER_BUFFERS] = { 0 };
         static void *zeros[PIPE_MAX_SAMPLERS] = { NULL };
         struct pipe_screen *scr = ctx->base.pipe->screen;
         enum pipe_shader_type sh;
         for (sh = 0; sh < PIPE_SHADER_MESH_TYPES; sh++) {
            switch (sh) {
            case PIPE_SHADER_GEOMETRY:
               if (!ctx->has_geometry_shader)
                  continue;
               break;
            case PIPE_SHADER_TESS_CTRL:
            case PIPE_SHADER_TESS_EVAL:
               if (!ctx->has_tessellation)
                  continue;
               break;
            case PIPE_SHADER_COMPUTE:
               if (!ctx->has_compute_shader)
                  continue;
               break;
            case PIPE_SHADER_MESH:
            case PIPE_SHADER_TASK:
               if (!ctx->has_task_mesh_shader)
                  continue;
               break;
            default:
               break;
            }

            int maxsam = scr->get_shader_param(scr, sh,
                                               PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS);
            int maxview = scr->get_shader_param(scr, sh,
                                                PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS);
            int maxssbo = scr->get_shader_param(scr, sh,
                                                PIPE_SHADER_CAP_MAX_SHADER_BUFFERS);
            int maxcb = scr->get_shader_param(scr, sh,
                                              PIPE_SHADER_CAP_MAX_CONST_BUFFERS);
            int maximg = scr->get_shader_param(scr, sh,
                                              PIPE_SHADER_CAP_MAX_SHADER_IMAGES);
            assert(maxsam <= PIPE_MAX_SAMPLERS);
            assert(maxview <= PIPE_MAX_SHADER_SAMPLER_VIEWS);
            assert(maxssbo <= PIPE_MAX_SHADER_BUFFERS);
            assert(maxcb <= PIPE_MAX_CONSTANT_BUFFERS);
            assert(maximg <= PIPE_MAX_SHADER_IMAGES);
            if (maxsam > 0) {
               ctx->base.pipe->bind_sampler_states(ctx->base.pipe, sh, 0, maxsam, zeros);
            }
            if (maxview > 0) {
               ctx->base.pipe->set_sampler_views(ctx->base.pipe, sh, 0, maxview, 0, false, views);
            }
            if (maxssbo > 0) {
               ctx->base.pipe->set_shader_buffers(ctx->base.pipe, sh, 0, maxssbo, ssbos, 0);
            }
            if (maximg > 0) {
               ctx->base.pipe->set_shader_images(ctx->base.pipe, sh, 0, 0, maximg, NULL);
            }
            for (int i = 0; i < maxcb; i++) {
               ctx->base.pipe->set_constant_buffer(ctx->base.pipe, sh, i, false, NULL);
            }
         }
      }

      ctx->base.pipe->bind_depth_stencil_alpha_state(ctx->base.pipe, NULL);
      struct pipe_stencil_ref sr = {0};
      ctx->base.pipe->set_stencil_ref(ctx->base.pipe, sr);
      ctx->base.pipe->bind_fs_state(ctx->base.pipe, NULL);
      ctx->base.pipe->set_constant_buffer(ctx->base.pipe, PIPE_SHADER_FRAGMENT, 0, false, NULL);
      ctx->base.pipe->bind_vs_state(ctx->base.pipe, NULL);
      ctx->base.pipe->set_constant_buffer(ctx->base.pipe, PIPE_SHADER_VERTEX, 0, false, NULL);
      if (ctx->has_geometry_shader) {
         ctx->base.pipe->bind_gs_state(ctx->base.pipe, NULL);
      }
      if (ctx->has_tessellation) {
         ctx->base.pipe->bind_tcs_state(ctx->base.pipe, NULL);
         ctx->base.pipe->bind_tes_state(ctx->base.pipe, NULL);
      }
      if (ctx->has_compute_shader) {
         ctx->base.pipe->bind_compute_state(ctx->base.pipe, NULL);
      }
      if (ctx->has_task_mesh_shader) {
         ctx->base.pipe->bind_ts_state(ctx->base.pipe, NULL);
         ctx->base.pipe->bind_ms_state(ctx->base.pipe, NULL);
      }
      ctx->base.pipe->bind_vertex_elements_state(ctx->base.pipe, NULL);

      if (ctx->has_streamout)
         ctx->base.pipe->set_stream_output_targets(ctx->base.pipe, 0, NULL, NULL);

      struct pipe_framebuffer_state fb = {0};
      ctx->base.pipe->set_framebuffer_state(ctx->base.pipe, &fb);
   }

   util_unreference_framebuffer_state(&ctx->fb);
   util_unreference_framebuffer_state(&ctx->fb_saved);

   for (i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
      pipe_so_target_reference(&ctx->so_targets[i], NULL);
      pipe_so_target_reference(&ctx->so_targets_saved[i], NULL);
   }

   memset(&ctx->samplers, 0, sizeof(ctx->samplers));
   memset(&ctx->nr_so_targets, 0,
          offsetof(struct cso_context_priv, cache)
          - offsetof(struct cso_context_priv, nr_so_targets));
   ctx->sample_mask = ~0;
   /*
    * If the cso context is reused (with the same pipe context),
    * need to really make sure the context state doesn't get out of sync.
    */
   ctx->base.pipe->set_sample_mask(ctx->base.pipe, ctx->sample_mask);
   if (ctx->base.pipe->set_min_samples)
      ctx->base.pipe->set_min_samples(ctx->base.pipe, ctx->min_samples);
   if (dumping)
      trace_dumping_start_locked();
}


/**
 * Free the CSO context.
 */
void
cso_destroy_context(struct cso_context *cso)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   cso_unbind_context(cso);
   cso_cache_delete(&ctx->cache);

   if (ctx->vbuf)
      u_vbuf_destroy(ctx->vbuf);

   ctx->base.pipe->vbuf = NULL;
   FREE(ctx);
}


/* Those function will either find the state of the given template
 * in the cache or they will create a new state from the given
 * template, insert it in the cache and return it.
 */

#define CSO_BLEND_KEY_SIZE_RT0      offsetof(struct pipe_blend_state, rt[1])
#define CSO_BLEND_KEY_SIZE_ALL_RT   sizeof(struct pipe_blend_state)

/*
 * If the driver returns 0 from the create method then they will assign
 * the data member of the cso to be the template itself.
 */

enum pipe_error
cso_set_blend(struct cso_context *cso,
              const struct pipe_blend_state *templ)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   unsigned key_size, hash_key;
   struct cso_hash_iter iter;
   void *handle;

   if (templ->independent_blend_enable) {
      /* This is duplicated with the else block below because we want key_size
       * to be a literal constant, so that memcpy and the hash computation can
       * be inlined and unrolled.
       */
      hash_key = cso_construct_key(templ, CSO_BLEND_KEY_SIZE_ALL_RT);
      iter = cso_find_state_template(&ctx->cache, hash_key, CSO_BLEND,
                                     templ, CSO_BLEND_KEY_SIZE_ALL_RT);
      key_size = CSO_BLEND_KEY_SIZE_ALL_RT;
   } else {
      hash_key = cso_construct_key(templ, CSO_BLEND_KEY_SIZE_RT0);
      iter = cso_find_state_template(&ctx->cache, hash_key, CSO_BLEND,
                                     templ, CSO_BLEND_KEY_SIZE_RT0);
      key_size = CSO_BLEND_KEY_SIZE_RT0;
   }

   if (cso_hash_iter_is_null(iter)) {
      struct cso_blend *cso = MALLOC(sizeof(struct cso_blend));
      if (!cso)
         return PIPE_ERROR_OUT_OF_MEMORY;

      memset(&cso->state, 0, sizeof cso->state);
      memcpy(&cso->state, templ, key_size);
      cso->data = ctx->base.pipe->create_blend_state(ctx->base.pipe, &cso->state);

      iter = cso_insert_state(&ctx->cache, hash_key, CSO_BLEND, cso);
      if (cso_hash_iter_is_null(iter)) {
         FREE(cso);
         return PIPE_ERROR_OUT_OF_MEMORY;
      }

      handle = cso->data;
   } else {
      handle = ((struct cso_blend *)cso_hash_iter_data(iter))->data;
   }

   if (ctx->blend != handle) {
      ctx->blend = handle;
      ctx->base.pipe->bind_blend_state(ctx->base.pipe, handle);
   }
   return PIPE_OK;
}


static void
cso_save_blend(struct cso_context_priv *ctx)
{
   assert(!ctx->blend_saved);
   ctx->blend_saved = ctx->blend;
}


static void
cso_restore_blend(struct cso_context_priv *ctx)
{
   if (ctx->blend != ctx->blend_saved) {
      ctx->blend = ctx->blend_saved;
      ctx->base.pipe->bind_blend_state(ctx->base.pipe, ctx->blend_saved);
   }
   ctx->blend_saved = NULL;
}


enum pipe_error
cso_set_depth_stencil_alpha(struct cso_context *cso,
                            const struct pipe_depth_stencil_alpha_state *templ)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   const unsigned key_size = sizeof(struct pipe_depth_stencil_alpha_state);
   const unsigned hash_key = cso_construct_key(templ, key_size);
   struct cso_hash_iter iter = cso_find_state_template(&ctx->cache,
                                                       hash_key,
                                                       CSO_DEPTH_STENCIL_ALPHA,
                                                       templ, key_size);
   void *handle;

   if (cso_hash_iter_is_null(iter)) {
      struct cso_depth_stencil_alpha *cso =
         MALLOC(sizeof(struct cso_depth_stencil_alpha));
      if (!cso)
         return PIPE_ERROR_OUT_OF_MEMORY;

      memcpy(&cso->state, templ, sizeof(*templ));
      cso->data = ctx->base.pipe->create_depth_stencil_alpha_state(ctx->base.pipe,
                                                              &cso->state);

      iter = cso_insert_state(&ctx->cache, hash_key,
                              CSO_DEPTH_STENCIL_ALPHA, cso);
      if (cso_hash_iter_is_null(iter)) {
         FREE(cso);
         return PIPE_ERROR_OUT_OF_MEMORY;
      }

      handle = cso->data;
   } else {
      handle = ((struct cso_depth_stencil_alpha *)
                cso_hash_iter_data(iter))->data;
   }

   if (ctx->depth_stencil != handle) {
      ctx->depth_stencil = handle;
      ctx->base.pipe->bind_depth_stencil_alpha_state(ctx->base.pipe, handle);
   }
   return PIPE_OK;
}


static void
cso_save_depth_stencil_alpha(struct cso_context_priv *ctx)
{
   assert(!ctx->depth_stencil_saved);
   ctx->depth_stencil_saved = ctx->depth_stencil;
}


static void
cso_restore_depth_stencil_alpha(struct cso_context_priv *ctx)
{
   if (ctx->depth_stencil != ctx->depth_stencil_saved) {
      ctx->depth_stencil = ctx->depth_stencil_saved;
      ctx->base.pipe->bind_depth_stencil_alpha_state(ctx->base.pipe,
                                                ctx->depth_stencil_saved);
   }
   ctx->depth_stencil_saved = NULL;
}


enum pipe_error
cso_set_rasterizer(struct cso_context *cso,
                   const struct pipe_rasterizer_state *templ)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   const unsigned key_size = sizeof(struct pipe_rasterizer_state);
   const unsigned hash_key = cso_construct_key(templ, key_size);
   struct cso_hash_iter iter = cso_find_state_template(&ctx->cache,
                                                       hash_key,
                                                       CSO_RASTERIZER,
                                                       templ, key_size);
   void *handle = NULL;

   /* We can't have both point_quad_rasterization (sprites) and point_smooth
    * (round AA points) enabled at the same time.
    */
   assert(!(templ->point_quad_rasterization && templ->point_smooth));

   if (cso_hash_iter_is_null(iter)) {
      struct cso_rasterizer *cso = MALLOC(sizeof(struct cso_rasterizer));
      if (!cso)
         return PIPE_ERROR_OUT_OF_MEMORY;

      memcpy(&cso->state, templ, sizeof(*templ));
      cso->data = ctx->base.pipe->create_rasterizer_state(ctx->base.pipe, &cso->state);

      iter = cso_insert_state(&ctx->cache, hash_key, CSO_RASTERIZER, cso);
      if (cso_hash_iter_is_null(iter)) {
         FREE(cso);
         return PIPE_ERROR_OUT_OF_MEMORY;
      }

      handle = cso->data;
   } else {
      handle = ((struct cso_rasterizer *)cso_hash_iter_data(iter))->data;
   }

   if (ctx->rasterizer != handle) {
      ctx->rasterizer = handle;
      ctx->flatshade_first = templ->flatshade_first;
      if (ctx->vbuf)
         u_vbuf_set_flatshade_first(ctx->vbuf, ctx->flatshade_first);
      ctx->base.pipe->bind_rasterizer_state(ctx->base.pipe, handle);
   }
   return PIPE_OK;
}


static void
cso_save_rasterizer(struct cso_context_priv *ctx)
{
   assert(!ctx->rasterizer_saved);
   ctx->rasterizer_saved = ctx->rasterizer;
   ctx->flatshade_first_saved = ctx->flatshade_first;
}


static void
cso_restore_rasterizer(struct cso_context_priv *ctx)
{
   if (ctx->rasterizer != ctx->rasterizer_saved) {
      ctx->rasterizer = ctx->rasterizer_saved;
      ctx->flatshade_first = ctx->flatshade_first_saved;
      if (ctx->vbuf)
         u_vbuf_set_flatshade_first(ctx->vbuf, ctx->flatshade_first);
      ctx->base.pipe->bind_rasterizer_state(ctx->base.pipe, ctx->rasterizer_saved);
   }
   ctx->rasterizer_saved = NULL;
}


void
cso_set_fragment_shader_handle(struct cso_context *cso, void *handle)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   if (ctx->fragment_shader != handle) {
      ctx->fragment_shader = handle;
      ctx->base.pipe->bind_fs_state(ctx->base.pipe, handle);
   }
}


static void
cso_save_fragment_shader(struct cso_context_priv *ctx)
{
   assert(!ctx->fragment_shader_saved);
   ctx->fragment_shader_saved = ctx->fragment_shader;
}


static void
cso_restore_fragment_shader(struct cso_context_priv *ctx)
{
   if (ctx->fragment_shader_saved != ctx->fragment_shader) {
      ctx->base.pipe->bind_fs_state(ctx->base.pipe, ctx->fragment_shader_saved);
      ctx->fragment_shader = ctx->fragment_shader_saved;
   }
   ctx->fragment_shader_saved = NULL;
}


void
cso_set_vertex_shader_handle(struct cso_context *cso, void *handle)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   if (ctx->vertex_shader != handle) {
      ctx->vertex_shader = handle;
      ctx->base.pipe->bind_vs_state(ctx->base.pipe, handle);
   }
}


static void
cso_save_vertex_shader(struct cso_context_priv *ctx)
{
   assert(!ctx->vertex_shader_saved);
   ctx->vertex_shader_saved = ctx->vertex_shader;
}


static void
cso_restore_vertex_shader(struct cso_context_priv *ctx)
{
   if (ctx->vertex_shader_saved != ctx->vertex_shader) {
      ctx->base.pipe->bind_vs_state(ctx->base.pipe, ctx->vertex_shader_saved);
      ctx->vertex_shader = ctx->vertex_shader_saved;
   }
   ctx->vertex_shader_saved = NULL;
}


void
cso_set_framebuffer(struct cso_context *cso,
                    const struct pipe_framebuffer_state *fb)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   if (memcmp(&ctx->fb, fb, sizeof(*fb)) != 0) {
      util_copy_framebuffer_state(&ctx->fb, fb);
      ctx->base.pipe->set_framebuffer_state(ctx->base.pipe, fb);
   }
}


static void
cso_save_framebuffer(struct cso_context_priv *ctx)
{
   util_copy_framebuffer_state(&ctx->fb_saved, &ctx->fb);
}


static void
cso_restore_framebuffer(struct cso_context_priv *ctx)
{
   if (memcmp(&ctx->fb, &ctx->fb_saved, sizeof(ctx->fb))) {
      util_copy_framebuffer_state(&ctx->fb, &ctx->fb_saved);
      ctx->base.pipe->set_framebuffer_state(ctx->base.pipe, &ctx->fb);
      util_unreference_framebuffer_state(&ctx->fb_saved);
   }
}


void
cso_set_viewport(struct cso_context *cso,
                 const struct pipe_viewport_state *vp)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   if (memcmp(&ctx->vp, vp, sizeof(*vp))) {
      ctx->vp = *vp;
      ctx->base.pipe->set_viewport_states(ctx->base.pipe, 0, 1, vp);
   }
}


/**
 * Setup viewport state for given width and height (position is always (0,0)).
 * Invert the Y axis if 'invert' is true.
 */
void
cso_set_viewport_dims(struct cso_context *ctx,
                      float width, float height, bool invert)
{
   struct pipe_viewport_state vp;
   vp.scale[0] = width * 0.5f;
   vp.scale[1] = height * (invert ? -0.5f : 0.5f);
   vp.scale[2] = 0.5f;
   vp.translate[0] = 0.5f * width;
   vp.translate[1] = 0.5f * height;
   vp.translate[2] = 0.5f;
   vp.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
   vp.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
   vp.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
   vp.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;
   cso_set_viewport(ctx, &vp);
}


static void
cso_save_viewport(struct cso_context_priv *ctx)
{
   ctx->vp_saved = ctx->vp;
}


static void
cso_restore_viewport(struct cso_context_priv *ctx)
{
   if (memcmp(&ctx->vp, &ctx->vp_saved, sizeof(ctx->vp))) {
      ctx->vp = ctx->vp_saved;
      ctx->base.pipe->set_viewport_states(ctx->base.pipe, 0, 1, &ctx->vp);
   }
}


void
cso_set_sample_mask(struct cso_context *cso, unsigned sample_mask)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   if (ctx->sample_mask != sample_mask) {
      ctx->sample_mask = sample_mask;
      ctx->base.pipe->set_sample_mask(ctx->base.pipe, sample_mask);
   }
}


static void
cso_save_sample_mask(struct cso_context_priv *ctx)
{
   ctx->sample_mask_saved = ctx->sample_mask;
}


static void
cso_restore_sample_mask(struct cso_context_priv *ctx)
{
   cso_set_sample_mask(&ctx->base, ctx->sample_mask_saved);
}


void
cso_set_min_samples(struct cso_context *cso, unsigned min_samples)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   if (ctx->min_samples != min_samples && ctx->base.pipe->set_min_samples) {
      ctx->min_samples = min_samples;
      ctx->base.pipe->set_min_samples(ctx->base.pipe, min_samples);
   }
}


static void
cso_save_min_samples(struct cso_context_priv *ctx)
{
   ctx->min_samples_saved = ctx->min_samples;
}


static void
cso_restore_min_samples(struct cso_context_priv *ctx)
{
   cso_set_min_samples(&ctx->base, ctx->min_samples_saved);
}


void
cso_set_stencil_ref(struct cso_context *cso,
                    const struct pipe_stencil_ref sr)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   if (memcmp(&ctx->stencil_ref, &sr, sizeof(ctx->stencil_ref))) {
      ctx->stencil_ref = sr;
      ctx->base.pipe->set_stencil_ref(ctx->base.pipe, sr);
   }
}


static void
cso_save_stencil_ref(struct cso_context_priv *ctx)
{
   ctx->stencil_ref_saved = ctx->stencil_ref;
}


static void
cso_restore_stencil_ref(struct cso_context_priv *ctx)
{
   if (memcmp(&ctx->stencil_ref, &ctx->stencil_ref_saved,
              sizeof(ctx->stencil_ref))) {
      ctx->stencil_ref = ctx->stencil_ref_saved;
      ctx->base.pipe->set_stencil_ref(ctx->base.pipe, ctx->stencil_ref);
   }
}


void
cso_set_render_condition(struct cso_context *cso,
                         struct pipe_query *query,
                         bool condition,
                         enum pipe_render_cond_flag mode)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   struct pipe_context *pipe = ctx->base.pipe;

   if (ctx->render_condition != query ||
       ctx->render_condition_mode != mode ||
       ctx->render_condition_cond != condition) {
      pipe->render_condition(pipe, query, condition, mode);
      ctx->render_condition = query;
      ctx->render_condition_cond = condition;
      ctx->render_condition_mode = mode;
   }
}


static void
cso_save_render_condition(struct cso_context_priv *ctx)
{
   ctx->render_condition_saved = ctx->render_condition;
   ctx->render_condition_cond_saved = ctx->render_condition_cond;
   ctx->render_condition_mode_saved = ctx->render_condition_mode;
}


static void
cso_restore_render_condition(struct cso_context_priv *ctx)
{
   cso_set_render_condition(&ctx->base, ctx->render_condition_saved,
                            ctx->render_condition_cond_saved,
                            ctx->render_condition_mode_saved);
}


void
cso_set_geometry_shader_handle(struct cso_context *cso, void *handle)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   assert(ctx->has_geometry_shader || !handle);

   if (ctx->has_geometry_shader && ctx->geometry_shader != handle) {
      ctx->geometry_shader = handle;
      ctx->base.pipe->bind_gs_state(ctx->base.pipe, handle);
   }
}


static void
cso_save_geometry_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_geometry_shader) {
      return;
   }

   assert(!ctx->geometry_shader_saved);
   ctx->geometry_shader_saved = ctx->geometry_shader;
}


static void
cso_restore_geometry_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_geometry_shader) {
      return;
   }

   if (ctx->geometry_shader_saved != ctx->geometry_shader) {
      ctx->base.pipe->bind_gs_state(ctx->base.pipe, ctx->geometry_shader_saved);
      ctx->geometry_shader = ctx->geometry_shader_saved;
   }
   ctx->geometry_shader_saved = NULL;
}


void
cso_set_tessctrl_shader_handle(struct cso_context *cso, void *handle)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   assert(ctx->has_tessellation || !handle);

   if (ctx->has_tessellation && ctx->tessctrl_shader != handle) {
      ctx->tessctrl_shader = handle;
      ctx->base.pipe->bind_tcs_state(ctx->base.pipe, handle);
   }
}


static void
cso_save_tessctrl_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_tessellation) {
      return;
   }

   assert(!ctx->tessctrl_shader_saved);
   ctx->tessctrl_shader_saved = ctx->tessctrl_shader;
}


static void
cso_restore_tessctrl_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_tessellation) {
      return;
   }

   if (ctx->tessctrl_shader_saved != ctx->tessctrl_shader) {
      ctx->base.pipe->bind_tcs_state(ctx->base.pipe, ctx->tessctrl_shader_saved);
      ctx->tessctrl_shader = ctx->tessctrl_shader_saved;
   }
   ctx->tessctrl_shader_saved = NULL;
}


void
cso_set_tesseval_shader_handle(struct cso_context *cso, void *handle)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   assert(ctx->has_tessellation || !handle);

   if (ctx->has_tessellation && ctx->tesseval_shader != handle) {
      ctx->tesseval_shader = handle;
      ctx->base.pipe->bind_tes_state(ctx->base.pipe, handle);
   }
}


static void
cso_save_tesseval_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_tessellation) {
      return;
   }

   assert(!ctx->tesseval_shader_saved);
   ctx->tesseval_shader_saved = ctx->tesseval_shader;
}


static void
cso_restore_tesseval_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_tessellation) {
      return;
   }

   if (ctx->tesseval_shader_saved != ctx->tesseval_shader) {
      ctx->base.pipe->bind_tes_state(ctx->base.pipe, ctx->tesseval_shader_saved);
      ctx->tesseval_shader = ctx->tesseval_shader_saved;
   }
   ctx->tesseval_shader_saved = NULL;
}


void
cso_set_compute_shader_handle(struct cso_context *cso, void *handle)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   assert(ctx->has_compute_shader || !handle);

   if (ctx->has_compute_shader && ctx->compute_shader != handle) {
      ctx->compute_shader = handle;
      ctx->base.pipe->bind_compute_state(ctx->base.pipe, handle);
   }
}


static void
cso_save_compute_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_compute_shader) {
      return;
   }

   assert(!ctx->compute_shader_saved);
   ctx->compute_shader_saved = ctx->compute_shader;
}


static void
cso_restore_compute_shader(struct cso_context_priv *ctx)
{
   if (!ctx->has_compute_shader) {
      return;
   }

   if (ctx->compute_shader_saved != ctx->compute_shader) {
      ctx->base.pipe->bind_compute_state(ctx->base.pipe, ctx->compute_shader_saved);
      ctx->compute_shader = ctx->compute_shader_saved;
   }
   ctx->compute_shader_saved = NULL;
}


static void
cso_save_compute_samplers(struct cso_context_priv *ctx)
{
   struct sampler_info *info = &ctx->samplers[PIPE_SHADER_COMPUTE];
   struct sampler_info *saved = &ctx->compute_samplers_saved;

   memcpy(saved->cso_samplers, info->cso_samplers,
          sizeof(info->cso_samplers));
   memcpy(saved->samplers, info->samplers, sizeof(info->samplers));
}


static void
cso_restore_compute_samplers(struct cso_context_priv *ctx)
{
   struct sampler_info *info = &ctx->samplers[PIPE_SHADER_COMPUTE];
   struct sampler_info *saved = &ctx->compute_samplers_saved;

   memcpy(info->cso_samplers, saved->cso_samplers,
          sizeof(info->cso_samplers));
   memcpy(info->samplers, saved->samplers, sizeof(info->samplers));

   for (int i = PIPE_MAX_SAMPLERS - 1; i >= 0; i--) {
      if (info->samplers[i]) {
         ctx->max_sampler_seen = i;
         break;
      }
   }

   cso_single_sampler_done(&ctx->base, PIPE_SHADER_COMPUTE);
}


static void
cso_set_vertex_elements_direct(struct cso_context_priv *ctx,
                               const struct cso_velems_state *velems)
{
   /* Need to include the count into the stored state data too.
    * Otherwise first few count pipe_vertex_elements could be identical
    * even if count is different, and there's no guarantee the hash would
    * be different in that case neither.
    */
   const unsigned key_size =
      sizeof(struct pipe_vertex_element) * velems->count + sizeof(unsigned);
   const unsigned hash_key = cso_construct_key((void*)velems, key_size);
   struct cso_hash_iter iter =
      cso_find_state_template(&ctx->cache, hash_key, CSO_VELEMENTS,
                              velems, key_size);
   void *handle;

   if (cso_hash_iter_is_null(iter)) {
      struct cso_velements *cso = MALLOC(sizeof(struct cso_velements));
      if (!cso)
         return;

      memcpy(&cso->state, velems, key_size);

      /* Lower 64-bit vertex attributes. */
      unsigned new_count = velems->count;
      const struct pipe_vertex_element *new_elems = velems->velems;
      struct pipe_vertex_element tmp[PIPE_MAX_ATTRIBS];
      util_lower_uint64_vertex_elements(&new_elems, &new_count, tmp);

      cso->data = ctx->base.pipe->create_vertex_elements_state(ctx->base.pipe, new_count,
                                                          new_elems);

      iter = cso_insert_state(&ctx->cache, hash_key, CSO_VELEMENTS, cso);
      if (cso_hash_iter_is_null(iter)) {
         FREE(cso);
         return;
      }

      handle = cso->data;
   } else {
      handle = ((struct cso_velements *)cso_hash_iter_data(iter))->data;
   }

   if (ctx->velements != handle) {
      ctx->velements = handle;
      ctx->base.pipe->bind_vertex_elements_state(ctx->base.pipe, handle);
   }
}


enum pipe_error
cso_set_vertex_elements(struct cso_context *cso,
                        const struct cso_velems_state *velems)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   struct u_vbuf *vbuf = ctx->vbuf_current;

   if (vbuf) {
      u_vbuf_set_vertex_elements(vbuf, velems);
      return PIPE_OK;
   }

   cso_set_vertex_elements_direct(ctx, velems);
   return PIPE_OK;
}


static void
cso_save_vertex_elements(struct cso_context_priv *ctx)
{
   struct u_vbuf *vbuf = ctx->vbuf_current;

   if (vbuf) {
      u_vbuf_save_vertex_elements(vbuf);
      return;
   }

   assert(!ctx->velements_saved);
   ctx->velements_saved = ctx->velements;
}


static void
cso_restore_vertex_elements(struct cso_context_priv *ctx)
{
   struct u_vbuf *vbuf = ctx->vbuf_current;

   if (vbuf) {
      u_vbuf_restore_vertex_elements(vbuf);
      return;
   }

   if (ctx->velements != ctx->velements_saved) {
      ctx->velements = ctx->velements_saved;
      ctx->base.pipe->bind_vertex_elements_state(ctx->base.pipe, ctx->velements_saved);
   }
   ctx->velements_saved = NULL;
}

/* vertex buffers */

void
cso_set_vertex_buffers(struct cso_context *cso,
                       unsigned count,
                       unsigned unbind_trailing_count,
                       bool take_ownership,
                       const struct pipe_vertex_buffer *buffers)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   struct u_vbuf *vbuf = ctx->vbuf_current;

   if (!count && !unbind_trailing_count)
      return;

   if (vbuf) {
      u_vbuf_set_vertex_buffers(vbuf, count, unbind_trailing_count,
                                take_ownership, buffers);
      return;
   }

   struct pipe_context *pipe = ctx->base.pipe;
   pipe->set_vertex_buffers(pipe, count, unbind_trailing_count,
                            take_ownership, buffers);
}


/**
 * Set vertex buffers and vertex elements. Skip u_vbuf if it's only needed
 * for user vertex buffers and user vertex buffers are not set by this call.
 * u_vbuf will be disabled. To re-enable u_vbuf, call this function again.
 *
 * Skipping u_vbuf decreases CPU overhead for draw calls that don't need it,
 * such as VBOs, glBegin/End, and display lists.
 *
 * Internal operations that do "save states, draw, restore states" shouldn't
 * use this, because the states are only saved in either cso_context or
 * u_vbuf, not both.
 */
void
cso_set_vertex_buffers_and_elements(struct cso_context *cso,
                                    const struct cso_velems_state *velems,
                                    unsigned vb_count,
                                    unsigned unbind_trailing_vb_count,
                                    bool take_ownership,
                                    bool uses_user_vertex_buffers,
                                    const struct pipe_vertex_buffer *vbuffers)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   struct u_vbuf *vbuf = ctx->vbuf;
   struct pipe_context *pipe = ctx->base.pipe;

   if (vbuf && (ctx->always_use_vbuf || uses_user_vertex_buffers)) {
      if (!ctx->vbuf_current) {
         /* Unbind all buffers in cso_context, because we'll use u_vbuf. */
         unsigned unbind_vb_count = vb_count + unbind_trailing_vb_count;
         if (unbind_vb_count)
            pipe->set_vertex_buffers(pipe, 0, unbind_vb_count, false, NULL);

         /* Unset this to make sure the CSO is re-bound on the next use. */
         ctx->velements = NULL;
         ctx->vbuf_current = pipe->vbuf = vbuf;
         if (pipe->draw_vbo == tc_draw_vbo)
            ctx->base.draw_vbo = u_vbuf_draw_vbo;
         unbind_trailing_vb_count = 0;
      }

      if (vb_count || unbind_trailing_vb_count) {
         u_vbuf_set_vertex_buffers(vbuf, vb_count,
                                   unbind_trailing_vb_count,
                                   take_ownership, vbuffers);
      }
      u_vbuf_set_vertex_elements(vbuf, velems);
      return;
   }

   if (ctx->vbuf_current) {
      /* Unbind all buffers in u_vbuf, because we'll use cso_context. */
      unsigned unbind_vb_count = vb_count + unbind_trailing_vb_count;
      if (unbind_vb_count)
         u_vbuf_set_vertex_buffers(vbuf, 0, unbind_vb_count, false, NULL);

      /* Unset this to make sure the CSO is re-bound on the next use. */
      u_vbuf_unset_vertex_elements(vbuf);
      ctx->vbuf_current = pipe->vbuf = NULL;
      if (pipe->draw_vbo == tc_draw_vbo)
         ctx->base.draw_vbo = pipe->draw_vbo;
      unbind_trailing_vb_count = 0;
   }

   if (vb_count || unbind_trailing_vb_count) {
      pipe->set_vertex_buffers(pipe, vb_count, unbind_trailing_vb_count,
                               take_ownership, vbuffers);
   }
   cso_set_vertex_elements_direct(ctx, velems);
}


ALWAYS_INLINE static struct cso_sampler *
set_sampler(struct cso_context_priv *ctx, enum pipe_shader_type shader_stage,
            unsigned idx, const struct pipe_sampler_state *templ,
            size_t key_size)
{
   unsigned hash_key = cso_construct_key(templ, key_size);
   struct cso_sampler *cso;
   struct cso_hash_iter iter =
      cso_find_state_template(&ctx->cache,
                              hash_key, CSO_SAMPLER,
                              templ, key_size);

   if (cso_hash_iter_is_null(iter)) {
      cso = MALLOC(sizeof(struct cso_sampler));
      if (!cso)
         return false;

      memcpy(&cso->state, templ, sizeof(*templ));
      cso->data = ctx->base.pipe->create_sampler_state(ctx->base.pipe, &cso->state);
      cso->hash_key = hash_key;

      iter = cso_insert_state(&ctx->cache, hash_key, CSO_SAMPLER, cso);
      if (cso_hash_iter_is_null(iter)) {
         FREE(cso);
         return false;
      }
   } else {
      cso = cso_hash_iter_data(iter);
   }
   return cso;
}


ALWAYS_INLINE static bool
cso_set_sampler(struct cso_context_priv *ctx, enum pipe_shader_type shader_stage,
                unsigned idx, const struct pipe_sampler_state *templ,
                size_t size)
{
   struct cso_sampler *cso = set_sampler(ctx, shader_stage, idx, templ, size);
   ctx->samplers[shader_stage].cso_samplers[idx] = cso;
   ctx->samplers[shader_stage].samplers[idx] = cso->data;
   return true;
}


void
cso_single_sampler(struct cso_context *cso, enum pipe_shader_type shader_stage,
                   unsigned idx, const struct pipe_sampler_state *templ)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;

   /* The reasons both blocks are duplicated is that we want the size parameter
    * to be a constant expression to inline and unroll memcmp and hash key
    * computations.
    */
   if (ctx->sampler_format) {
      if (cso_set_sampler(ctx, shader_stage, idx, templ,
                          sizeof(struct pipe_sampler_state)))
         ctx->max_sampler_seen = MAX2(ctx->max_sampler_seen, (int)idx);
   } else {
      if (cso_set_sampler(ctx, shader_stage, idx, templ,
                          offsetof(struct pipe_sampler_state, border_color_format)))
         ctx->max_sampler_seen = MAX2(ctx->max_sampler_seen, (int)idx);
   }
}


/**
 * Send staged sampler state to the driver.
 */
void
cso_single_sampler_done(struct cso_context *cso,
                        enum pipe_shader_type shader_stage)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   struct sampler_info *info = &ctx->samplers[shader_stage];

   if (ctx->max_sampler_seen == -1)
      return;

   ctx->base.pipe->bind_sampler_states(ctx->base.pipe, shader_stage, 0,
                                  ctx->max_sampler_seen + 1,
                                  info->samplers);
   ctx->max_sampler_seen = -1;
}


ALWAYS_INLINE static int
set_samplers(struct cso_context_priv *ctx,
             enum pipe_shader_type shader_stage,
             unsigned nr,
             const struct pipe_sampler_state **templates,
             size_t key_size)
{
   int last = -1;
   for (unsigned i = 0; i < nr; i++) {
      if (!templates[i])
         continue;

      /* Reuse the same sampler state CSO if 2 consecutive sampler states
       * are identical.
       *
       * The trivial case where both pointers are equal doesn't occur in
       * frequented codepaths.
       *
       * Reuse rate:
       * - Borderlands 2: 55%
       * - Hitman: 65%
       * - Rocket League: 75%
       * - Tomb Raider: 50-65%
       * - XCOM 2: 55%
       */
      if (last >= 0 &&
          !memcmp(templates[i], templates[last],
                  key_size)) {
         ctx->samplers[shader_stage].cso_samplers[i] =
            ctx->samplers[shader_stage].cso_samplers[last];
         ctx->samplers[shader_stage].samplers[i] =
            ctx->samplers[shader_stage].samplers[last];
      } else {
         /* Look up the sampler state CSO. */
         cso_set_sampler(ctx, shader_stage, i, templates[i], key_size);
      }

      last = i;
   }
   return last;
}


/*
 * If the function encouters any errors it will return the
 * last one. Done to always try to set as many samplers
 * as possible.
 */
void
cso_set_samplers(struct cso_context *cso,
                 enum pipe_shader_type shader_stage,
                 unsigned nr,
                 const struct pipe_sampler_state **templates)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   int last;

   /* ensure sampler size is a constant for memcmp */
   if (ctx->sampler_format) {
      last = set_samplers(ctx, shader_stage, nr, templates,
                          sizeof(struct pipe_sampler_state));
   } else {
      last = set_samplers(ctx, shader_stage, nr, templates,
                          offsetof(struct pipe_sampler_state, border_color_format));
   }

   ctx->max_sampler_seen = MAX2(ctx->max_sampler_seen, last);
   cso_single_sampler_done(&ctx->base, shader_stage);
}


static void
cso_save_fragment_samplers(struct cso_context_priv *ctx)
{
   struct sampler_info *info = &ctx->samplers[PIPE_SHADER_FRAGMENT];
   struct sampler_info *saved = &ctx->fragment_samplers_saved;

   memcpy(saved->cso_samplers, info->cso_samplers,
          sizeof(info->cso_samplers));
   memcpy(saved->samplers, info->samplers, sizeof(info->samplers));
}


static void
cso_restore_fragment_samplers(struct cso_context_priv *ctx)
{
   struct sampler_info *info = &ctx->samplers[PIPE_SHADER_FRAGMENT];
   struct sampler_info *saved = &ctx->fragment_samplers_saved;

   memcpy(info->cso_samplers, saved->cso_samplers,
          sizeof(info->cso_samplers));
   memcpy(info->samplers, saved->samplers, sizeof(info->samplers));

   for (int i = PIPE_MAX_SAMPLERS - 1; i >= 0; i--) {
      if (info->samplers[i]) {
         ctx->max_sampler_seen = i;
         break;
      }
   }

   cso_single_sampler_done(&ctx->base, PIPE_SHADER_FRAGMENT);
}


void
cso_set_stream_outputs(struct cso_context *cso,
                       unsigned num_targets,
                       struct pipe_stream_output_target **targets,
                       const unsigned *offsets)
{
   struct cso_context_priv *ctx = (struct cso_context_priv *)cso;
   struct pipe_context *pipe = ctx->base.pipe;
   unsigned i;

   if (!ctx->has_streamout) {
      assert(num_targets == 0);
      return;
   }

   if (ctx->nr_so_targets == 0 && num_targets == 0) {
      /* Nothing to do. */
      return;
   }

   /* reference new targets */
   for (i = 0; i < num_targets; i++) {
      pipe_so_target_reference(&ctx->so_targets[i], targets[i]);
   }
   /* unref extra old targets, if any */
   for (; i < ctx->nr_so_targets; i++) {
      pipe_so_target_reference(&ctx->so_targets[i], NULL);
   }

   pipe->set_stream_output_targets(pipe, num_targets, targets,
                                   offsets);
   ctx->nr_so_targets = num_targets;
}


static void
cso_save_stream_outputs(struct cso_context_priv *ctx)
{
   if (!ctx->has_streamout) {
      return;
   }

   ctx->nr_so_targets_saved = ctx->nr_so_targets;

   for (unsigned i = 0; i < ctx->nr_so_targets; i++) {
      assert(!ctx->so_targets_saved[i]);
      pipe_so_target_reference(&ctx->so_targets_saved[i], ctx->so_targets[i]);
   }
}


static void
cso_restore_stream_outputs(struct cso_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;
   unsigned i;
   unsigned offset[PIPE_MAX_SO_BUFFERS];

   if (!ctx->has_streamout) {
      return;
   }

   if (ctx->nr_so_targets == 0 && ctx->nr_so_targets_saved == 0) {
      /* Nothing to do. */
      return;
   }

   assert(ctx->nr_so_targets_saved <= PIPE_MAX_SO_BUFFERS);
   for (i = 0; i < ctx->nr_so_targets_saved; i++) {
      pipe_so_target_reference(&ctx->so_targets[i], NULL);
      /* move the reference from one pointer to another */
      ctx->so_targets[i] = ctx->so_targets_saved[i];
      ctx->so_targets_saved[i] = NULL;
      /* -1 means append */
      offset[i] = (unsigned)-1;
   }
   for (; i < ctx->nr_so_targets; i++) {
      pipe_so_target_reference(&ctx->so_targets[i], NULL);
   }

   pipe->set_stream_output_targets(pipe, ctx->nr_so_targets_saved,
                                   ctx->so_targets, offset);

   ctx->nr_so_targets = ctx->nr_so_targets_saved;
   ctx->nr_so_targets_saved = 0;
}


/**
 * Save all the CSO state items specified by the state_mask bitmask
 * of CSO_BIT_x flags.
 */
void
cso_save_state(struct cso_context *ctx, unsigned state_mask)
{
   struct cso_context_priv *cso = (struct cso_context_priv *)ctx;
   assert(cso->saved_state == 0);

   cso->saved_state = state_mask;

   if (state_mask & CSO_BIT_BLEND)
      cso_save_blend(cso);
   if (state_mask & CSO_BIT_DEPTH_STENCIL_ALPHA)
      cso_save_depth_stencil_alpha(cso);
   if (state_mask & CSO_BIT_FRAGMENT_SAMPLERS)
      cso_save_fragment_samplers(cso);
   if (state_mask & CSO_BIT_FRAGMENT_SHADER)
      cso_save_fragment_shader(cso);
   if (state_mask & CSO_BIT_FRAMEBUFFER)
      cso_save_framebuffer(cso);
   if (state_mask & CSO_BIT_GEOMETRY_SHADER)
      cso_save_geometry_shader(cso);
   if (state_mask & CSO_BIT_MIN_SAMPLES)
      cso_save_min_samples(cso);
   if (state_mask & CSO_BIT_RASTERIZER)
      cso_save_rasterizer(cso);
   if (state_mask & CSO_BIT_RENDER_CONDITION)
      cso_save_render_condition(cso);
   if (state_mask & CSO_BIT_SAMPLE_MASK)
      cso_save_sample_mask(cso);
   if (state_mask & CSO_BIT_STENCIL_REF)
      cso_save_stencil_ref(cso);
   if (state_mask & CSO_BIT_STREAM_OUTPUTS)
      cso_save_stream_outputs(cso);
   if (state_mask & CSO_BIT_TESSCTRL_SHADER)
      cso_save_tessctrl_shader(cso);
   if (state_mask & CSO_BIT_TESSEVAL_SHADER)
      cso_save_tesseval_shader(cso);
   if (state_mask & CSO_BIT_VERTEX_ELEMENTS)
      cso_save_vertex_elements(cso);
   if (state_mask & CSO_BIT_VERTEX_SHADER)
      cso_save_vertex_shader(cso);
   if (state_mask & CSO_BIT_VIEWPORT)
      cso_save_viewport(cso);
   if (state_mask & CSO_BIT_PAUSE_QUERIES)
      cso->base.pipe->set_active_query_state(cso->base.pipe, false);
}


/**
 * Restore the state which was saved by cso_save_state().
 */
void
cso_restore_state(struct cso_context *ctx, unsigned unbind)
{
   struct cso_context_priv *cso = (struct cso_context_priv *)ctx;
   unsigned state_mask = cso->saved_state;

   assert(state_mask);

   if (state_mask & CSO_BIT_DEPTH_STENCIL_ALPHA)
      cso_restore_depth_stencil_alpha(cso);
   if (state_mask & CSO_BIT_STENCIL_REF)
      cso_restore_stencil_ref(cso);
   if (state_mask & CSO_BIT_FRAGMENT_SHADER)
      cso_restore_fragment_shader(cso);
   if (state_mask & CSO_BIT_GEOMETRY_SHADER)
      cso_restore_geometry_shader(cso);
   if (state_mask & CSO_BIT_TESSEVAL_SHADER)
      cso_restore_tesseval_shader(cso);
   if (state_mask & CSO_BIT_TESSCTRL_SHADER)
      cso_restore_tessctrl_shader(cso);
   if (state_mask & CSO_BIT_VERTEX_SHADER)
      cso_restore_vertex_shader(cso);
   if (unbind & CSO_UNBIND_FS_SAMPLERVIEWS)
      cso->base.pipe->set_sampler_views(cso->base.pipe, PIPE_SHADER_FRAGMENT, 0, 0,
                                   cso->max_fs_samplerviews, false, NULL);
   if (unbind & CSO_UNBIND_FS_SAMPLERVIEW0)
      cso->base.pipe->set_sampler_views(cso->base.pipe, PIPE_SHADER_FRAGMENT, 0, 0,
                                   1, false, NULL);
   if (state_mask & CSO_BIT_FRAGMENT_SAMPLERS)
      cso_restore_fragment_samplers(cso);
   if (unbind & CSO_UNBIND_FS_IMAGE0)
      cso->base.pipe->set_shader_images(cso->base.pipe, PIPE_SHADER_FRAGMENT, 0, 0, 1, NULL);
   if (state_mask & CSO_BIT_FRAMEBUFFER)
      cso_restore_framebuffer(cso);
   if (state_mask & CSO_BIT_BLEND)
      cso_restore_blend(cso);
   if (state_mask & CSO_BIT_RASTERIZER)
      cso_restore_rasterizer(cso);
   if (state_mask & CSO_BIT_MIN_SAMPLES)
      cso_restore_min_samples(cso);
   if (state_mask & CSO_BIT_RENDER_CONDITION)
      cso_restore_render_condition(cso);
   if (state_mask & CSO_BIT_SAMPLE_MASK)
      cso_restore_sample_mask(cso);
   if (state_mask & CSO_BIT_VIEWPORT)
      cso_restore_viewport(cso);
   if (unbind & CSO_UNBIND_VS_CONSTANTS)
      cso->base.pipe->set_constant_buffer(cso->base.pipe, PIPE_SHADER_VERTEX, 0, false, NULL);
   if (unbind & CSO_UNBIND_FS_CONSTANTS)
      cso->base.pipe->set_constant_buffer(cso->base.pipe, PIPE_SHADER_FRAGMENT, 0, false, NULL);
   if (state_mask & CSO_BIT_VERTEX_ELEMENTS)
      cso_restore_vertex_elements(cso);
   if (unbind & CSO_UNBIND_VERTEX_BUFFER0)
      cso->base.pipe->set_vertex_buffers(cso->base.pipe, 0, 1, false, NULL);
   if (state_mask & CSO_BIT_STREAM_OUTPUTS)
      cso_restore_stream_outputs(cso);
   if (state_mask & CSO_BIT_PAUSE_QUERIES)
      cso->base.pipe->set_active_query_state(cso->base.pipe, true);

   cso->saved_state = 0;
}


/**
 * Save all the CSO state items specified by the state_mask bitmask
 * of CSO_BIT_COMPUTE_x flags.
 */
void
cso_save_compute_state(struct cso_context *ctx, unsigned state_mask)
{
   struct cso_context_priv *cso = (struct cso_context_priv *)ctx;
   assert(cso->saved_compute_state == 0);

   cso->saved_compute_state = state_mask;

   if (state_mask & CSO_BIT_COMPUTE_SHADER)
      cso_save_compute_shader(cso);

   if (state_mask & CSO_BIT_COMPUTE_SAMPLERS)
      cso_save_compute_samplers(cso);
}


/**
 * Restore the state which was saved by cso_save_compute_state().
 */
void
cso_restore_compute_state(struct cso_context *ctx)
{
   struct cso_context_priv *cso = (struct cso_context_priv *)ctx;
   unsigned state_mask = cso->saved_compute_state;

   assert(state_mask);

   if (state_mask & CSO_BIT_COMPUTE_SHADER)
      cso_restore_compute_shader(cso);

   if (state_mask & CSO_BIT_COMPUTE_SAMPLERS)
      cso_restore_compute_samplers(cso);

   cso->saved_compute_state = 0;
}



/* drawing */

void
cso_draw_arrays(struct cso_context *ctx, unsigned mode, unsigned start, unsigned count)
{
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias draw;

   util_draw_init_info(&info);

   info.mode = mode;
   info.index_bounds_valid = true;
   info.min_index = start;
   info.max_index = start + count - 1;

   draw.start = start;
   draw.count = count;
   draw.index_bias = 0;

   cso_draw_vbo(ctx, &info, 0, NULL, &draw, 1);
}


void
cso_draw_arrays_instanced(struct cso_context *ctx, unsigned mode,
                          unsigned start, unsigned count,
                          unsigned start_instance, unsigned instance_count)
{
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias draw;

   util_draw_init_info(&info);

   info.mode = mode;
   info.index_bounds_valid = true;
   info.min_index = start;
   info.max_index = start + count - 1;
   info.start_instance = start_instance;
   info.instance_count = instance_count;

   draw.start = start;
   draw.count = count;
   draw.index_bias = 0;

   cso_draw_vbo(ctx, &info, 0, NULL, &draw, 1);
}
