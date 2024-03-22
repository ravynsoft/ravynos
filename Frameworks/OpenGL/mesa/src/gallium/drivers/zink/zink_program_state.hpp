/*
 * Copyright © 2022 Valve Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 * 
 * Authors:
 *    Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 */


/**
 * this file is used to optimize pipeline state management
 * pipeline state comparisons are the most significant cause of CPU overhead aside from descriptors,
 * so more effort must be taken to reduce it by any means
 */
#include "zink_types.h"
#include "zink_pipeline.h"
#include "zink_program.h"
#include "zink_screen.h"

/* runtime-optimized pipeline state hashing */
template <zink_dynamic_state DYNAMIC_STATE>
static uint32_t
hash_gfx_pipeline_state(const void *key, struct zink_screen *screen)
{
   const struct zink_gfx_pipeline_state *state = (const struct zink_gfx_pipeline_state *)key;
   uint32_t hash = _mesa_hash_data(key, screen->have_full_ds3 ?
                                        offsetof(struct zink_gfx_pipeline_state, sample_mask) :
                                        offsetof(struct zink_gfx_pipeline_state, hash));
   if (DYNAMIC_STATE < ZINK_DYNAMIC_STATE2)
      hash = XXH32(&state->dyn_state3, sizeof(state->dyn_state3), hash);
   if (DYNAMIC_STATE < ZINK_DYNAMIC_STATE3)
      hash = XXH32(&state->dyn_state2, sizeof(state->dyn_state2), hash);
   if (DYNAMIC_STATE != ZINK_NO_DYNAMIC_STATE)
      return hash;
   return XXH32(&state->dyn_state1, sizeof(state->dyn_state1), hash);
}

template <bool HAS_DYNAMIC>
static unsigned
get_pipeline_idx(enum mesa_prim mode, VkPrimitiveTopology vkmode)
{
   /* VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY specifies that the topology state in
    * VkPipelineInputAssemblyStateCreateInfo only specifies the topology class,
    * and the specific topology order and adjacency must be set dynamically
    * with vkCmdSetPrimitiveTopology before any drawing commands.
    */
   if (HAS_DYNAMIC) {
      return get_primtype_idx(mode);
   }
   return vkmode;
}

/*
   VUID-vkCmdBindVertexBuffers2-pStrides-06209
   If pStrides is not NULL each element of pStrides must be either 0 or greater than or equal
   to the maximum extent of all vertex input attributes fetched from the corresponding
   binding, where the extent is calculated as the VkVertexInputAttributeDescription::offset
   plus VkVertexInputAttributeDescription::format size

   * thus, if the stride doesn't meet the minimum requirement for a binding,
   * disable the dynamic state here and use a fully-baked pipeline
 */
static bool
check_vertex_strides(struct zink_context *ctx)
{
   const struct zink_vertex_elements_state *ves = ctx->element_state;
   for (unsigned i = 0; i < ves->hw_state.num_bindings; i++) {
      const struct pipe_vertex_buffer *vb = ctx->vertex_buffers + ves->hw_state.binding_map[i];
      unsigned stride = vb->buffer.resource ? ves->hw_state.b.strides[i] : 0;
      if (stride && stride < ves->min_stride[i])
         return false;
   }
   return true;
}

/* runtime-optimized function to recalc pipeline state and find a usable pipeline:
 * in theory, zink supports many feature levels,
 * but it's important to provide a more optimized codepath for drivers that support all the best features
 */
template <zink_dynamic_state DYNAMIC_STATE, bool HAVE_LIB>
VkPipeline
zink_get_gfx_pipeline(struct zink_context *ctx,
                      struct zink_gfx_program *prog,
                      struct zink_gfx_pipeline_state *state,
                      enum mesa_prim mode)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   bool uses_dynamic_stride = state->uses_dynamic_stride;

   VkPrimitiveTopology vkmode = zink_primitive_topology(mode);
   const unsigned idx = screen->info.dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted ?
                        0 :
                        get_pipeline_idx<DYNAMIC_STATE >= ZINK_DYNAMIC_STATE>(mode, vkmode);
   assert(idx <= ARRAY_SIZE(prog->pipelines[0]));
   if (!state->dirty && !state->modules_changed &&
       ((DYNAMIC_STATE == ZINK_DYNAMIC_VERTEX_INPUT || DYNAMIC_STATE == ZINK_DYNAMIC_VERTEX_INPUT2) && !ctx->vertex_state_changed) &&
       idx == state->idx)
      return state->pipeline;

   struct hash_entry *entry = NULL;

   /* recalc the base pipeline state hash */
   if (state->dirty) {
      if (state->pipeline) //avoid on first hash
         state->final_hash ^= state->hash;
      state->hash = hash_gfx_pipeline_state<DYNAMIC_STATE>(state, screen);
      state->final_hash ^= state->hash;
      state->dirty = false;
   }
   /* extra safety asserts for optimal path to catch refactoring bugs */
   if (prog->optimal_keys) {
      ASSERTED const union zink_shader_key_optimal *opt = (union zink_shader_key_optimal*)&prog->last_variant_hash;
      ASSERTED uint32_t sanitized = zink_sanitize_optimal_key(ctx->gfx_stages, ctx->gfx_pipeline_state.shader_keys_optimal.key.val);
      assert(opt->val == sanitized);
      assert(state->optimal_key == sanitized);
   }
   /* recalc vertex state if missing optimal extensions */
   if (DYNAMIC_STATE != ZINK_DYNAMIC_VERTEX_INPUT2 && DYNAMIC_STATE != ZINK_DYNAMIC_VERTEX_INPUT && ctx->vertex_state_changed) {
      if (state->pipeline)
         state->final_hash ^= state->vertex_hash;
      /* even if dynamic stride is available, it may not be usable with the current pipeline */
      if (DYNAMIC_STATE != ZINK_NO_DYNAMIC_STATE)
         uses_dynamic_stride = check_vertex_strides(ctx);
      if (!uses_dynamic_stride) {
         uint32_t hash = 0;
         /* if we don't have dynamic states, we have to hash the enabled vertex buffer bindings */
         uint32_t vertex_buffers_enabled_mask = state->vertex_buffers_enabled_mask;
         hash = XXH32(&vertex_buffers_enabled_mask, sizeof(uint32_t), hash);

         for (unsigned i = 0; i < state->element_state->num_bindings; i++) {
            const unsigned buffer_id = ctx->element_state->hw_state.binding_map[i];
            struct pipe_vertex_buffer *vb = ctx->vertex_buffers + buffer_id;
            state->vertex_strides[buffer_id] = vb->buffer.resource ? state->element_state->b.strides[i] : 0;
            hash = XXH32(&state->vertex_strides[buffer_id], sizeof(uint32_t), hash);
         }
         state->vertex_hash = hash ^ state->element_state->hash;
      } else
         state->vertex_hash = state->element_state->hash;
      state->final_hash ^= state->vertex_hash;
   }
   state->modules_changed = false;
   state->uses_dynamic_stride = uses_dynamic_stride;
   state->idx = idx;
   ctx->vertex_state_changed = false;

   const int rp_idx = state->render_pass ? 1 : 0;
   /* shortcut for reusing previous pipeline across program changes */
   if (DYNAMIC_STATE == ZINK_DYNAMIC_VERTEX_INPUT || DYNAMIC_STATE == ZINK_DYNAMIC_VERTEX_INPUT2) {
      if (prog->last_finalized_hash[rp_idx][idx] == state->final_hash &&
          !prog->inline_variants && likely(prog->last_pipeline[rp_idx][idx]) &&
          /* this data is too big to compare in the fast-path */
          likely(!prog->shaders[MESA_SHADER_FRAGMENT]->fs.legacy_shadow_mask)) {
         state->pipeline = prog->last_pipeline[rp_idx][idx];
         return state->pipeline;
      }
   }
   entry = _mesa_hash_table_search_pre_hashed(&prog->pipelines[rp_idx][idx], state->final_hash, state);

   if (!entry) {
      /* always wait on async precompile/cache fence */
      util_queue_fence_wait(&prog->base.cache_fence);
      struct zink_gfx_pipeline_cache_entry *pc_entry = CALLOC_STRUCT(zink_gfx_pipeline_cache_entry);
      if (!pc_entry)
         return VK_NULL_HANDLE;
      /* cache entries must have all state needed to construct pipelines
       * TODO: maybe optimize this since all these values aren't actually needed
       */
      memcpy(&pc_entry->state, state, sizeof(*state));
      pc_entry->state.rendering_info.pColorAttachmentFormats = pc_entry->state.rendering_formats;
      pc_entry->prog = prog;
      /* init the optimized background compile fence */
      util_queue_fence_init(&pc_entry->fence);
      entry = _mesa_hash_table_insert_pre_hashed(&prog->pipelines[rp_idx][idx], state->final_hash, pc_entry, pc_entry);
      if (prog->base.uses_shobj && !prog->is_separable) {
         memcpy(pc_entry->shobjs, prog->objs, sizeof(prog->objs));
         zink_gfx_program_compile_queue(ctx, pc_entry);
      } else if (HAVE_LIB && zink_can_use_pipeline_libs(ctx)) {
         /* this is the graphics pipeline library path: find/construct all partial pipelines */
         simple_mtx_lock(&prog->libs->lock);
         struct set_entry *he = _mesa_set_search(&prog->libs->libs, &ctx->gfx_pipeline_state.optimal_key);
         struct zink_gfx_library_key *gkey;
         if (he) {
            gkey = (struct zink_gfx_library_key *)he->key;
         } else {
            assert(!prog->is_separable);
            gkey = zink_create_pipeline_lib(screen, prog, &ctx->gfx_pipeline_state);
         }
         simple_mtx_unlock(&prog->libs->lock);
         struct zink_gfx_input_key *ikey = DYNAMIC_STATE == ZINK_DYNAMIC_VERTEX_INPUT ?
                                             zink_find_or_create_input_dynamic(ctx, vkmode) :
                                             zink_find_or_create_input(ctx, vkmode);
         struct zink_gfx_output_key *okey = DYNAMIC_STATE >= ZINK_DYNAMIC_STATE3 && screen->have_full_ds3 ?
                                             zink_find_or_create_output_ds3(ctx) :
                                             zink_find_or_create_output(ctx);
         /* partial pipelines are stored to the cache entry for async optimized pipeline compiles */
         pc_entry->gpl.ikey = ikey;
         pc_entry->gpl.gkey = gkey;
         pc_entry->gpl.okey = okey;
         /* try to hit optimized compile cache first if possible */
         if (!prog->is_separable)
            pc_entry->pipeline = zink_create_gfx_pipeline_combined(screen, prog, ikey->pipeline, &gkey->pipeline, 1, okey->pipeline, true, true);
         if (!pc_entry->pipeline) {
            /* create the non-optimized pipeline first using fast-linking to avoid stuttering */
            pc_entry->pipeline = zink_create_gfx_pipeline_combined(screen, prog, ikey->pipeline, &gkey->pipeline, 1, okey->pipeline, false, false);
            if (!prog->is_separable)
               /* trigger async optimized pipeline compile if this was the fast-linked unoptimized pipeline */
               zink_gfx_program_compile_queue(ctx, pc_entry);
         }
      } else {
         /* optimize by default only when expecting precompiles in order to reduce stuttering */
         if (DYNAMIC_STATE != ZINK_DYNAMIC_VERTEX_INPUT2 && DYNAMIC_STATE != ZINK_DYNAMIC_VERTEX_INPUT)
            pc_entry->pipeline = zink_create_gfx_pipeline(screen, prog, prog->objs, state, state->element_state->binding_map, vkmode, !HAVE_LIB, NULL);
         else
            pc_entry->pipeline = zink_create_gfx_pipeline(screen, prog, prog->objs, state, NULL, vkmode, !HAVE_LIB, NULL);
         if (HAVE_LIB && !prog->is_separable)
            /* trigger async optimized pipeline compile if this was an unoptimized pipeline */
            zink_gfx_program_compile_queue(ctx, pc_entry);
      }
      if (pc_entry->pipeline == VK_NULL_HANDLE)
         return VK_NULL_HANDLE;

      zink_screen_update_pipeline_cache(screen, &prog->base, false);
   }

   struct zink_gfx_pipeline_cache_entry *cache_entry = (struct zink_gfx_pipeline_cache_entry *)entry->data;
   state->pipeline = cache_entry->pipeline;
   /* update states for fastpath */
   if (DYNAMIC_STATE >= ZINK_DYNAMIC_VERTEX_INPUT) {
      prog->last_finalized_hash[rp_idx][idx] = state->final_hash;
      prog->last_pipeline[rp_idx][idx] = cache_entry->pipeline;
   }
   return state->pipeline;
}

/* runtime-optimized pipeline state comparisons */
template <zink_pipeline_dynamic_state DYNAMIC_STATE, unsigned STAGE_MASK>
static bool
equals_gfx_pipeline_state(const void *a, const void *b)
{
   const struct zink_gfx_pipeline_state *sa = (const struct zink_gfx_pipeline_state *)a;
   const struct zink_gfx_pipeline_state *sb = (const struct zink_gfx_pipeline_state *)b;
   if (DYNAMIC_STATE < ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT) {
      if (sa->uses_dynamic_stride != sb->uses_dynamic_stride)
         return false;
   }
   if (DYNAMIC_STATE == ZINK_PIPELINE_NO_DYNAMIC_STATE ||
       (DYNAMIC_STATE < ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT && !sa->uses_dynamic_stride)) {
      if (sa->vertex_buffers_enabled_mask != sb->vertex_buffers_enabled_mask)
         return false;
      /* if we don't have dynamic states, we have to hash the enabled vertex buffer bindings */
      uint32_t mask_a = sa->vertex_buffers_enabled_mask;
      uint32_t mask_b = sb->vertex_buffers_enabled_mask;
      while (mask_a || mask_b) {
         unsigned idx_a = u_bit_scan(&mask_a);
         unsigned idx_b = u_bit_scan(&mask_b);
         if (sa->vertex_strides[idx_a] != sb->vertex_strides[idx_b])
            return false;
      }
   }

   /* each dynamic state extension has its own struct on the pipeline state to compare
    * if all extensions are supported, none of them are accessed
    */
   if (DYNAMIC_STATE == ZINK_PIPELINE_NO_DYNAMIC_STATE) {
      if (memcmp(&sa->dyn_state1, &sb->dyn_state1, offsetof(struct zink_pipeline_dynamic_state1, depth_stencil_alpha_state)))
         return false;
      if (!!sa->dyn_state1.depth_stencil_alpha_state != !!sb->dyn_state1.depth_stencil_alpha_state ||
          (sa->dyn_state1.depth_stencil_alpha_state &&
           memcmp(sa->dyn_state1.depth_stencil_alpha_state, sb->dyn_state1.depth_stencil_alpha_state,
                  sizeof(struct zink_depth_stencil_alpha_hw_state))))
         return false;
   }
   if (DYNAMIC_STATE < ZINK_PIPELINE_DYNAMIC_STATE3) {
      if (DYNAMIC_STATE < ZINK_PIPELINE_DYNAMIC_STATE2) {
         if (memcmp(&sa->dyn_state2, &sb->dyn_state2, sizeof(sa->dyn_state2)))
            return false;
      }
      if (memcmp(&sa->dyn_state3, &sb->dyn_state3, sizeof(sa->dyn_state3)))
         return false;
   } else if (DYNAMIC_STATE != ZINK_PIPELINE_DYNAMIC_STATE2_PCP &&
              DYNAMIC_STATE != ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT2_PCP &&
              DYNAMIC_STATE != ZINK_PIPELINE_DYNAMIC_STATE3_PCP &&
              DYNAMIC_STATE != ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT_PCP &&
              (STAGE_MASK & BITFIELD_BIT(MESA_SHADER_TESS_EVAL)) &&
              !(STAGE_MASK & BITFIELD_BIT(MESA_SHADER_TESS_CTRL))) {
      if (sa->dyn_state2.vertices_per_patch != sb->dyn_state2.vertices_per_patch)
         return false;
   }
   /* optimal keys are the fastest path: only a single uint32_t comparison for all shader module variants */
   if (STAGE_MASK & STAGE_MASK_OPTIMAL) {
      if (sa->optimal_key != sb->optimal_key)
         return false;
      if (STAGE_MASK & STAGE_MASK_OPTIMAL_SHADOW) {
         if (sa->shadow != sb->shadow)
            return false;
      }
   } else {
      if (STAGE_MASK & BITFIELD_BIT(MESA_SHADER_TESS_CTRL)) {
         if (sa->modules[MESA_SHADER_TESS_CTRL] != sb->modules[MESA_SHADER_TESS_CTRL])
            return false;
      }
      if (STAGE_MASK & BITFIELD_BIT(MESA_SHADER_TESS_EVAL)) {
         if (sa->modules[MESA_SHADER_TESS_EVAL] != sb->modules[MESA_SHADER_TESS_EVAL])
            return false;
      }
      if (STAGE_MASK & BITFIELD_BIT(MESA_SHADER_GEOMETRY)) {
         if (sa->modules[MESA_SHADER_GEOMETRY] != sb->modules[MESA_SHADER_GEOMETRY])
            return false;
      }
      if (sa->modules[MESA_SHADER_VERTEX] != sb->modules[MESA_SHADER_VERTEX])
         return false;
      if (sa->modules[MESA_SHADER_FRAGMENT] != sb->modules[MESA_SHADER_FRAGMENT])
         return false;
   }
   /* the base pipeline state is a 12 byte comparison */
   return !memcmp(a, b, offsetof(struct zink_gfx_pipeline_state, hash));
}

/* below is a bunch of code to pick the right equals_gfx_pipeline_state template for runtime */
template <zink_pipeline_dynamic_state DYNAMIC_STATE, unsigned STAGE_MASK>
static equals_gfx_pipeline_state_func
get_optimal_gfx_pipeline_stage_eq_func(bool optimal_keys, bool shadow_needs_shader_swizzle)
{
   if (optimal_keys) {
      if (shadow_needs_shader_swizzle)
         return equals_gfx_pipeline_state<DYNAMIC_STATE, STAGE_MASK | STAGE_MASK_OPTIMAL | STAGE_MASK_OPTIMAL_SHADOW>;
      return equals_gfx_pipeline_state<DYNAMIC_STATE, STAGE_MASK | STAGE_MASK_OPTIMAL>;
   }
   return equals_gfx_pipeline_state<DYNAMIC_STATE, STAGE_MASK>;
}

template <zink_pipeline_dynamic_state DYNAMIC_STATE>
static equals_gfx_pipeline_state_func
get_gfx_pipeline_stage_eq_func(struct zink_gfx_program *prog, bool optimal_keys)
{
   bool shadow_needs_shader_swizzle = prog->shaders[MESA_SHADER_FRAGMENT]->fs.legacy_shadow_mask > 0;
   unsigned vertex_stages = prog->stages_present & BITFIELD_MASK(MESA_SHADER_FRAGMENT);
   if (vertex_stages & BITFIELD_BIT(MESA_SHADER_TESS_CTRL)) {
      if (prog->shaders[MESA_SHADER_TESS_CTRL]->non_fs.is_generated)
         vertex_stages &= ~BITFIELD_BIT(MESA_SHADER_TESS_CTRL);
   }
   if (vertex_stages & BITFIELD_BIT(MESA_SHADER_TESS_CTRL)) {
      if (vertex_stages == BITFIELD_MASK(MESA_SHADER_FRAGMENT))
         /* all stages */
         return get_optimal_gfx_pipeline_stage_eq_func<DYNAMIC_STATE,
                                                       BITFIELD_MASK(MESA_SHADER_COMPUTE)>(optimal_keys, shadow_needs_shader_swizzle);
      if (vertex_stages == BITFIELD_MASK(MESA_SHADER_GEOMETRY))
         /* tess only: includes generated tcs too */
         return get_optimal_gfx_pipeline_stage_eq_func<DYNAMIC_STATE,
                                                       BITFIELD_MASK(MESA_SHADER_COMPUTE) & ~BITFIELD_BIT(MESA_SHADER_GEOMETRY)>(optimal_keys, shadow_needs_shader_swizzle);
      if (vertex_stages == (BITFIELD_BIT(MESA_SHADER_VERTEX) | BITFIELD_BIT(MESA_SHADER_GEOMETRY)))
         /* geom only */
         return get_optimal_gfx_pipeline_stage_eq_func<DYNAMIC_STATE,
                                                       BITFIELD_BIT(MESA_SHADER_VERTEX) | BITFIELD_BIT(MESA_SHADER_FRAGMENT) | BITFIELD_BIT(MESA_SHADER_GEOMETRY)>(optimal_keys, shadow_needs_shader_swizzle);
   }
   if (vertex_stages == (BITFIELD_MASK(MESA_SHADER_FRAGMENT) & ~BITFIELD_BIT(MESA_SHADER_TESS_CTRL)))
      /* all stages but tcs */
      return get_optimal_gfx_pipeline_stage_eq_func<DYNAMIC_STATE,
                                                    BITFIELD_MASK(MESA_SHADER_COMPUTE) & ~BITFIELD_BIT(MESA_SHADER_TESS_CTRL)>(optimal_keys, shadow_needs_shader_swizzle);
   if (vertex_stages == (BITFIELD_MASK(MESA_SHADER_GEOMETRY) & ~BITFIELD_BIT(MESA_SHADER_TESS_CTRL)))
      /* tess only: generated tcs */
      return get_optimal_gfx_pipeline_stage_eq_func<DYNAMIC_STATE,
                                                    BITFIELD_MASK(MESA_SHADER_COMPUTE) & ~(BITFIELD_BIT(MESA_SHADER_GEOMETRY) | BITFIELD_BIT(MESA_SHADER_TESS_CTRL))>(optimal_keys, shadow_needs_shader_swizzle);
   if (vertex_stages == (BITFIELD_BIT(MESA_SHADER_VERTEX) | BITFIELD_BIT(MESA_SHADER_GEOMETRY)))
      /* geom only */
      return get_optimal_gfx_pipeline_stage_eq_func<DYNAMIC_STATE,
                                                    BITFIELD_BIT(MESA_SHADER_VERTEX) | BITFIELD_BIT(MESA_SHADER_FRAGMENT) | BITFIELD_BIT(MESA_SHADER_GEOMETRY)>(optimal_keys, shadow_needs_shader_swizzle);
   return get_optimal_gfx_pipeline_stage_eq_func<DYNAMIC_STATE,
                                                 BITFIELD_BIT(MESA_SHADER_VERTEX) | BITFIELD_BIT(MESA_SHADER_FRAGMENT)>(optimal_keys, shadow_needs_shader_swizzle);
}

equals_gfx_pipeline_state_func
zink_get_gfx_pipeline_eq_func(struct zink_screen *screen, struct zink_gfx_program *prog)
{
   if (screen->info.have_EXT_extended_dynamic_state) {
      if (screen->info.have_EXT_extended_dynamic_state2) {
         if (screen->info.have_EXT_extended_dynamic_state3) {
            if (screen->info.have_EXT_vertex_input_dynamic_state) {
               if (screen->info.dynamic_state2_feats.extendedDynamicState2PatchControlPoints)
                  return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT_PCP>(prog, screen->optimal_keys);
               else
                  return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT>(prog, screen->optimal_keys);
            } else {
               if (screen->info.dynamic_state2_feats.extendedDynamicState2PatchControlPoints)
                  return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_STATE3_PCP>(prog, screen->optimal_keys);
               else
                  return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_STATE3>(prog, screen->optimal_keys);
            }
         }
         if (screen->info.have_EXT_vertex_input_dynamic_state) {
            if (screen->info.dynamic_state2_feats.extendedDynamicState2PatchControlPoints)
               return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT2_PCP>(prog, screen->optimal_keys);
            else
               return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT2>(prog, screen->optimal_keys);
         } else {
            if (screen->info.dynamic_state2_feats.extendedDynamicState2PatchControlPoints)
               return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_STATE2_PCP>(prog, screen->optimal_keys);
            else
               return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_STATE2>(prog, screen->optimal_keys);
         }
      }
      return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_DYNAMIC_STATE>(prog, screen->optimal_keys);
   }
   return get_gfx_pipeline_stage_eq_func<ZINK_PIPELINE_NO_DYNAMIC_STATE>(prog, screen->optimal_keys);
}
