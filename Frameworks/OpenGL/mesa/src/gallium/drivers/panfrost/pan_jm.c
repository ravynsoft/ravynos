/*
 * Copyright (C) 2018 Alyssa Rosenzweig
 * Copyright (C) 2020 Collabora Ltd.
 * Copyright © 2017 Intel Corporation
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
 */

#include "decode.h"

#include "drm-uapi/panfrost_drm.h"

#include "pan_blitter.h"
#include "pan_cmdstream.h"
#include "pan_context.h"
#include "pan_indirect_dispatch.h"
#include "pan_jm.h"
#include "pan_job.h"

#if PAN_ARCH >= 10
#error "JM helpers are only used for gen < 10"
#endif

void
GENX(jm_init_batch)(struct panfrost_batch *batch)
{
   /* Reserve the framebuffer and local storage descriptors */
   batch->framebuffer =
#if PAN_ARCH == 4
      pan_pool_alloc_desc(&batch->pool.base, FRAMEBUFFER);
#else
      pan_pool_alloc_desc_aggregate(
         &batch->pool.base, PAN_DESC(FRAMEBUFFER), PAN_DESC(ZS_CRC_EXTENSION),
         PAN_DESC_ARRAY(MAX2(batch->key.nr_cbufs, 1), RENDER_TARGET));
#endif

#if PAN_ARCH >= 6
   batch->tls = pan_pool_alloc_desc(&batch->pool.base, LOCAL_STORAGE);
#else
   /* On Midgard, the TLS is embedded in the FB descriptor */
   batch->tls = batch->framebuffer;

#if PAN_ARCH == 5
   struct mali_framebuffer_pointer_packed ptr;

   pan_pack(ptr.opaque, FRAMEBUFFER_POINTER, cfg) {
      cfg.pointer = batch->framebuffer.gpu;
      cfg.render_target_count = 1; /* a necessary lie */
   }

   batch->tls.gpu = ptr.opaque[0];
#endif
#endif
}

static int
jm_submit_jc(struct panfrost_batch *batch, mali_ptr first_job_desc,
             uint32_t reqs, uint32_t out_sync)
{
   struct panfrost_context *ctx = batch->ctx;
   struct pipe_context *gallium = (struct pipe_context *)ctx;
   struct panfrost_device *dev = pan_device(gallium->screen);
   struct drm_panfrost_submit submit = {
      0,
   };
   uint32_t in_syncs[1];
   uint32_t *bo_handles;
   int ret;

   /* If we trace, we always need a syncobj, so make one of our own if we
    * weren't given one to use. Remember that we did so, so we can free it
    * after we're done but preventing double-frees if we were given a
    * syncobj */

   if (!out_sync && dev->debug & (PAN_DBG_TRACE | PAN_DBG_SYNC))
      out_sync = ctx->syncobj;

   submit.out_sync = out_sync;
   submit.jc = first_job_desc;
   submit.requirements = reqs;

   if (ctx->in_sync_fd >= 0) {
      ret = drmSyncobjImportSyncFile(panfrost_device_fd(dev), ctx->in_sync_obj,
                                     ctx->in_sync_fd);
      assert(!ret);

      in_syncs[submit.in_sync_count++] = ctx->in_sync_obj;
      close(ctx->in_sync_fd);
      ctx->in_sync_fd = -1;
   }

   if (submit.in_sync_count)
      submit.in_syncs = (uintptr_t)in_syncs;

   bo_handles = calloc(panfrost_pool_num_bos(&batch->pool) +
                          panfrost_pool_num_bos(&batch->invisible_pool) +
                          batch->num_bos + 2,
                       sizeof(*bo_handles));
   assert(bo_handles);

   pan_bo_access *flags = util_dynarray_begin(&batch->bos);
   unsigned end_bo = util_dynarray_num_elements(&batch->bos, pan_bo_access);

   for (int i = 0; i < end_bo; ++i) {
      if (!flags[i])
         continue;

      assert(submit.bo_handle_count < batch->num_bos);
      bo_handles[submit.bo_handle_count++] = i;

      /* Update the BO access flags so that panfrost_bo_wait() knows
       * about all pending accesses.
       * We only keep the READ/WRITE info since this is all the BO
       * wait logic cares about.
       * We also preserve existing flags as this batch might not
       * be the first one to access the BO.
       */
      struct panfrost_bo *bo = pan_lookup_bo(dev, i);

      bo->gpu_access |= flags[i] & (PAN_BO_ACCESS_RW);
   }

   panfrost_pool_get_bo_handles(&batch->pool,
                                bo_handles + submit.bo_handle_count);
   submit.bo_handle_count += panfrost_pool_num_bos(&batch->pool);
   panfrost_pool_get_bo_handles(&batch->invisible_pool,
                                bo_handles + submit.bo_handle_count);
   submit.bo_handle_count += panfrost_pool_num_bos(&batch->invisible_pool);

   /* Add the tiler heap to the list of accessed BOs if the batch has at
    * least one tiler job. Tiler heap is written by tiler jobs and read
    * by fragment jobs (the polygon list is coming from this heap).
    */
   if (batch->jm.jobs.vtc_jc.first_tiler)
      bo_handles[submit.bo_handle_count++] =
         panfrost_bo_handle(dev->tiler_heap);

   /* Always used on Bifrost, occassionally used on Midgard */
   bo_handles[submit.bo_handle_count++] =
      panfrost_bo_handle(dev->sample_positions);

   submit.bo_handles = (u64)(uintptr_t)bo_handles;
   if (ctx->is_noop)
      ret = 0;
   else
      ret = drmIoctl(panfrost_device_fd(dev), DRM_IOCTL_PANFROST_SUBMIT, &submit);
   free(bo_handles);

   if (ret)
      return errno;

   /* Trace the job if we're doing that */
   if (dev->debug & (PAN_DBG_TRACE | PAN_DBG_SYNC)) {
      /* Wait so we can get errors reported back */
      drmSyncobjWait(panfrost_device_fd(dev), &out_sync, 1, INT64_MAX, 0, NULL);

      if (dev->debug & PAN_DBG_TRACE)
         pandecode_jc(dev->decode_ctx, submit.jc, panfrost_device_gpu_id(dev));

      if (dev->debug & PAN_DBG_DUMP)
         pandecode_dump_mappings(dev->decode_ctx);

      /* Jobs won't be complete if blackhole rendering, that's ok */
      if (!ctx->is_noop && dev->debug & PAN_DBG_SYNC)
         pandecode_abort_on_fault(dev->decode_ctx, submit.jc, panfrost_device_gpu_id(dev));
   }

   return 0;
}

/* Submit both vertex/tiler and fragment jobs for a batch, possibly with an
 * outsync corresponding to the later of the two (since there will be an
 * implicit dep between them) */

int
GENX(jm_submit_batch)(struct panfrost_batch *batch)
{
   struct pipe_screen *pscreen = batch->ctx->base.screen;
   struct panfrost_device *dev = pan_device(pscreen);
   bool has_draws = batch->jm.jobs.vtc_jc.first_job;
   bool has_tiler = batch->jm.jobs.vtc_jc.first_tiler;
   bool has_frag = panfrost_has_fragment_job(batch);
   uint32_t out_sync = batch->ctx->syncobj;
   int ret = 0;

   /* Take the submit lock to make sure no tiler jobs from other context
    * are inserted between our tiler and fragment jobs, failing to do that
    * might result in tiler heap corruption.
    */
   if (has_tiler)
      pthread_mutex_lock(&dev->submit_lock);

   if (has_draws) {
      ret = jm_submit_jc(batch, batch->jm.jobs.vtc_jc.first_job, 0,
                         has_frag ? 0 : out_sync);

      if (ret)
         goto done;
   }

   if (has_frag) {
      ret =
         jm_submit_jc(batch, batch->jm.jobs.frag, PANFROST_JD_REQ_FS, out_sync);
      if (ret)
         goto done;
   }

done:
   if (has_tiler)
      pthread_mutex_unlock(&dev->submit_lock);

   return ret;
}

void
GENX(jm_preload_fb)(struct panfrost_batch *batch, struct pan_fb_info *fb)
{
   GENX(pan_preload_fb)
   (&batch->pool.base, &batch->jm.jobs.vtc_jc, fb, batch->tls.gpu,
    PAN_ARCH >= 6 ? batch->tiler_ctx.bifrost : 0, NULL);
}

void
GENX(jm_emit_fragment_job)(struct panfrost_batch *batch,
                           const struct pan_fb_info *pfb)
{
   struct panfrost_ptr transfer =
      pan_pool_alloc_desc(&batch->pool.base, FRAGMENT_JOB);

   GENX(pan_emit_fragment_job)(pfb, batch->framebuffer.gpu, transfer.cpu);

   batch->jm.jobs.frag = transfer.gpu;
}

#if PAN_ARCH == 9
static void
jm_emit_shader_env(struct panfrost_batch *batch,
                   struct MALI_SHADER_ENVIRONMENT *cfg,
                   enum pipe_shader_type stage, mali_ptr shader_ptr)
{
   cfg->resources = panfrost_emit_resources(batch, stage);
   cfg->thread_storage = batch->tls.gpu;
   cfg->shader = shader_ptr;

   /* Each entry of FAU is 64-bits */
   cfg->fau = batch->push_uniforms[stage];
   cfg->fau_count = DIV_ROUND_UP(batch->nr_push_uniforms[stage], 2);
}
#endif

void
GENX(jm_launch_grid)(struct panfrost_batch *batch,
                     const struct pipe_grid_info *info)
{
   struct panfrost_ptr t = pan_pool_alloc_desc(&batch->pool.base, COMPUTE_JOB);

   /* Invoke according to the grid info */

   unsigned num_wg[3] = {info->grid[0], info->grid[1], info->grid[2]};

   if (info->indirect)
      num_wg[0] = num_wg[1] = num_wg[2] = 1;

#if PAN_ARCH <= 7
   panfrost_pack_work_groups_compute(
      pan_section_ptr(t.cpu, COMPUTE_JOB, INVOCATION), num_wg[0], num_wg[1],
      num_wg[2], info->block[0], info->block[1], info->block[2], false,
      info->indirect != NULL);

   pan_section_pack(t.cpu, COMPUTE_JOB, PARAMETERS, cfg) {
      cfg.job_task_split = util_logbase2_ceil(info->block[0] + 1) +
                           util_logbase2_ceil(info->block[1] + 1) +
                           util_logbase2_ceil(info->block[2] + 1);
   }

   pan_section_pack(t.cpu, COMPUTE_JOB, DRAW, cfg) {
      cfg.state = batch->rsd[PIPE_SHADER_COMPUTE];
      cfg.attributes = batch->attribs[PIPE_SHADER_COMPUTE];
      cfg.attribute_buffers = batch->attrib_bufs[PIPE_SHADER_COMPUTE];
      cfg.thread_storage = batch->tls.gpu;
      cfg.uniform_buffers = batch->uniform_buffers[PIPE_SHADER_COMPUTE];
      cfg.push_uniforms = batch->push_uniforms[PIPE_SHADER_COMPUTE];
      cfg.textures = batch->textures[PIPE_SHADER_COMPUTE];
      cfg.samplers = batch->samplers[PIPE_SHADER_COMPUTE];
   }

#if PAN_ARCH == 4
   pan_section_pack(t.cpu, COMPUTE_JOB, COMPUTE_PADDING, cfg)
      ;
#endif
#else
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_compiled_shader *cs = ctx->prog[PIPE_SHADER_COMPUTE];

   pan_section_pack(t.cpu, COMPUTE_JOB, PAYLOAD, cfg) {
      cfg.workgroup_size_x = info->block[0];
      cfg.workgroup_size_y = info->block[1];
      cfg.workgroup_size_z = info->block[2];

      cfg.workgroup_count_x = num_wg[0];
      cfg.workgroup_count_y = num_wg[1];
      cfg.workgroup_count_z = num_wg[2];

      jm_emit_shader_env(batch, &cfg.compute, PIPE_SHADER_COMPUTE,
                         batch->rsd[PIPE_SHADER_COMPUTE]);

      /* Workgroups may be merged if the shader does not use barriers
       * or shared memory. This condition is checked against the
       * static shared_size at compile-time. We need to check the
       * variable shared size at launch_grid time, because the
       * compiler doesn't know about that.
       */
      cfg.allow_merging_workgroups = cs->info.cs.allow_merging_workgroups &&
                                     (info->variable_shared_mem == 0);

      cfg.task_increment = 1;
      cfg.task_axis = MALI_TASK_AXIS_Z;
   }
#endif

   unsigned indirect_dep = 0;
#if PAN_GPU_INDIRECTS
   if (info->indirect) {
      struct pan_indirect_dispatch_info indirect = {
         .job = t.gpu,
         .indirect_dim = pan_resource(info->indirect)->image.data.bo->ptr.gpu +
                         info->indirect_offset,
         .num_wg_sysval =
            {
               batch->num_wg_sysval[0],
               batch->num_wg_sysval[1],
               batch->num_wg_sysval[2],
            },
      };

      indirect_dep = GENX(pan_indirect_dispatch_emit)(
         &batch->pool.base, &batch->jm.jobs.vtc_jc, &indirect);
   }
#endif

   pan_jc_add_job(&batch->pool.base, &batch->jm.jobs.vtc_jc,
                  MALI_JOB_TYPE_COMPUTE, true, false, indirect_dep, 0, &t,
                  false);
}

#if PAN_ARCH >= 6
static mali_ptr
jm_emit_tiler_desc(struct panfrost_batch *batch)
{
   struct panfrost_device *dev = pan_device(batch->ctx->base.screen);

   if (batch->tiler_ctx.bifrost)
      return batch->tiler_ctx.bifrost;

   struct panfrost_ptr t = pan_pool_alloc_desc(&batch->pool.base, TILER_HEAP);

   pan_pack(t.cpu, TILER_HEAP, heap) {
      heap.size = panfrost_bo_size(dev->tiler_heap);
      heap.base = dev->tiler_heap->ptr.gpu;
      heap.bottom = dev->tiler_heap->ptr.gpu;
      heap.top = dev->tiler_heap->ptr.gpu + panfrost_bo_size(dev->tiler_heap);
   }

   mali_ptr heap = t.gpu;
   unsigned max_levels = dev->tiler_features.max_levels;
   assert(max_levels >= 2);

   t = pan_pool_alloc_desc(&batch->pool.base, TILER_CONTEXT);
   pan_pack(t.cpu, TILER_CONTEXT, tiler) {
      /* TODO: Select hierarchy mask more effectively */
      tiler.hierarchy_mask = (max_levels >= 8) ? 0xFF : 0x28;

      /* For large framebuffers, disable the smallest bin size to
       * avoid pathological tiler memory usage. Required to avoid OOM
       * on dEQP-GLES31.functional.fbo.no_attachments.maximums.all on
       * Mali-G57.
       */
      if (MAX2(batch->key.width, batch->key.height) >= 4096)
         tiler.hierarchy_mask &= ~1;

      tiler.fb_width = batch->key.width;
      tiler.fb_height = batch->key.height;
      tiler.heap = heap;
      tiler.sample_pattern =
         pan_sample_pattern(util_framebuffer_get_num_samples(&batch->key));
#if PAN_ARCH >= 9
      tiler.first_provoking_vertex =
         pan_tristate_get(batch->first_provoking_vertex);
#endif
   }

   batch->tiler_ctx.bifrost = t.gpu;
   return batch->tiler_ctx.bifrost;
}
#endif

#if PAN_ARCH <= 7
static inline void
jm_emit_draw_descs(struct panfrost_batch *batch, struct MALI_DRAW *d,
                   enum pipe_shader_type st)
{
   d->offset_start = batch->ctx->offset_start;
   d->instance_size =
      batch->ctx->instance_count > 1 ? batch->ctx->padded_count : 1;

   d->uniform_buffers = batch->uniform_buffers[st];
   d->push_uniforms = batch->push_uniforms[st];
   d->textures = batch->textures[st];
   d->samplers = batch->samplers[st];
}

static void
jm_emit_vertex_draw(struct panfrost_batch *batch, void *section)
{
   pan_pack(section, DRAW, cfg) {
      cfg.state = batch->rsd[PIPE_SHADER_VERTEX];
      cfg.attributes = batch->attribs[PIPE_SHADER_VERTEX];
      cfg.attribute_buffers = batch->attrib_bufs[PIPE_SHADER_VERTEX];
      cfg.varyings = batch->varyings.vs;
      cfg.varying_buffers = cfg.varyings ? batch->varyings.bufs : 0;
      cfg.thread_storage = batch->tls.gpu;
      jm_emit_draw_descs(batch, &cfg, PIPE_SHADER_VERTEX);
   }
}

static void
jm_emit_vertex_job(struct panfrost_batch *batch,
                   const struct pipe_draw_info *info, void *invocation_template,
                   void *job)
{
   void *section = pan_section_ptr(job, COMPUTE_JOB, INVOCATION);
   memcpy(section, invocation_template, pan_size(INVOCATION));

   pan_section_pack(job, COMPUTE_JOB, PARAMETERS, cfg) {
      cfg.job_task_split = 5;
   }

   section = pan_section_ptr(job, COMPUTE_JOB, DRAW);
   jm_emit_vertex_draw(batch, section);

#if PAN_ARCH == 4
   pan_section_pack(job, COMPUTE_JOB, COMPUTE_PADDING, cfg)
      ;
#endif
}
#endif /* PAN_ARCH <= 7 */

static void
jm_emit_tiler_draw(void *out, struct panfrost_batch *batch, bool fs_required,
                   enum mesa_prim prim)
{
   struct panfrost_context *ctx = batch->ctx;
   struct pipe_rasterizer_state *rast = &ctx->rasterizer->base;
   bool polygon = (prim == MESA_PRIM_TRIANGLES);

   pan_pack(out, DRAW, cfg) {
      /*
       * From the Gallium documentation,
       * pipe_rasterizer_state::cull_face "indicates which faces of
       * polygons to cull". Points and lines are not considered
       * polygons and should be drawn even if all faces are culled.
       * The hardware does not take primitive type into account when
       * culling, so we need to do that check ourselves.
       */
      cfg.cull_front_face = polygon && (rast->cull_face & PIPE_FACE_FRONT);
      cfg.cull_back_face = polygon && (rast->cull_face & PIPE_FACE_BACK);
      cfg.front_face_ccw = rast->front_ccw;

      if (ctx->occlusion_query && ctx->active_queries) {
         if (ctx->occlusion_query->type == PIPE_QUERY_OCCLUSION_COUNTER)
            cfg.occlusion_query = MALI_OCCLUSION_MODE_COUNTER;
         else
            cfg.occlusion_query = MALI_OCCLUSION_MODE_PREDICATE;

         struct panfrost_resource *rsrc =
            pan_resource(ctx->occlusion_query->rsrc);
         cfg.occlusion = rsrc->image.data.bo->ptr.gpu;
         panfrost_batch_write_rsrc(ctx->batch, rsrc, PIPE_SHADER_FRAGMENT);
      }

#if PAN_ARCH >= 9
      struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];

      cfg.multisample_enable = rast->multisample;
      cfg.sample_mask = rast->multisample ? ctx->sample_mask : 0xFFFF;

      /* Use per-sample shading if required by API Also use it when a
       * blend shader is used with multisampling, as this is handled
       * by a single ST_TILE in the blend shader with the current
       * sample ID, requiring per-sample shading.
       */
      cfg.evaluate_per_sample =
         (rast->multisample &&
          ((ctx->min_samples > 1) || ctx->valhall_has_blend_shader));

      cfg.single_sampled_lines = !rast->multisample;

      cfg.vertex_array.packet = true;

      cfg.minimum_z = batch->minimum_z;
      cfg.maximum_z = batch->maximum_z;

      cfg.depth_stencil = batch->depth_stencil;

      if (fs_required) {
         bool has_oq = ctx->occlusion_query && ctx->active_queries;

         struct pan_earlyzs_state earlyzs = pan_earlyzs_get(
            fs->earlyzs, ctx->depth_stencil->writes_zs || has_oq,
            ctx->blend->base.alpha_to_coverage,
            ctx->depth_stencil->zs_always_passes);

         cfg.pixel_kill_operation = earlyzs.kill;
         cfg.zs_update_operation = earlyzs.update;

         cfg.allow_forward_pixel_to_kill =
            pan_allow_forward_pixel_to_kill(ctx, fs);
         cfg.allow_forward_pixel_to_be_killed = !fs->info.writes_global;

         /* Mask of render targets that may be written. A render
          * target may be written if the fragment shader writes
          * to it AND it actually exists. If the render target
          * doesn't actually exist, the blend descriptor will be
          * OFF so it may be omitted from the mask.
          *
          * Only set when there is a fragment shader, since
          * otherwise no colour updates are possible.
          */
         cfg.render_target_mask =
            (fs->info.outputs_written >> FRAG_RESULT_DATA0) & ctx->fb_rt_mask;

         /* Also use per-sample shading if required by the shader
          */
         cfg.evaluate_per_sample |= fs->info.fs.sample_shading;

         /* Unlike Bifrost, alpha-to-coverage must be included in
          * this identically-named flag. Confusing, isn't it?
          */
         cfg.shader_modifies_coverage = fs->info.fs.writes_coverage ||
                                        fs->info.fs.can_discard ||
                                        ctx->blend->base.alpha_to_coverage;

         /* Blend descriptors are only accessed by a BLEND
          * instruction on Valhall. It follows that if the
          * fragment shader is omitted, we may also emit the
          * blend descriptors.
          */
         cfg.blend = batch->blend;
         cfg.blend_count = MAX2(batch->key.nr_cbufs, 1);
         cfg.alpha_to_coverage = ctx->blend->base.alpha_to_coverage;

         cfg.overdraw_alpha0 = panfrost_overdraw_alpha(ctx, 0);
         cfg.overdraw_alpha1 = panfrost_overdraw_alpha(ctx, 1);

         jm_emit_shader_env(batch, &cfg.shader, PIPE_SHADER_FRAGMENT,
                            batch->rsd[PIPE_SHADER_FRAGMENT]);
      } else {
         /* These operations need to be FORCE to benefit from the
          * depth-only pass optimizations.
          */
         cfg.pixel_kill_operation = MALI_PIXEL_KILL_FORCE_EARLY;
         cfg.zs_update_operation = MALI_PIXEL_KILL_FORCE_EARLY;

         /* No shader and no blend => no shader or blend
          * reasons to disable FPK. The only FPK-related state
          * not covered is alpha-to-coverage which we don't set
          * without blend.
          */
         cfg.allow_forward_pixel_to_kill = true;

         /* No shader => no shader side effects */
         cfg.allow_forward_pixel_to_be_killed = true;

         /* Alpha isn't written so these are vacuous */
         cfg.overdraw_alpha0 = true;
         cfg.overdraw_alpha1 = true;
      }
#else
      cfg.position = batch->varyings.pos;
      cfg.state = batch->rsd[PIPE_SHADER_FRAGMENT];
      cfg.attributes = batch->attribs[PIPE_SHADER_FRAGMENT];
      cfg.attribute_buffers = batch->attrib_bufs[PIPE_SHADER_FRAGMENT];
      cfg.viewport = batch->viewport;
      cfg.varyings = batch->varyings.fs;
      cfg.varying_buffers = cfg.varyings ? batch->varyings.bufs : 0;
      cfg.thread_storage = batch->tls.gpu;

      /* For all primitives but lines DRAW.flat_shading_vertex must
       * be set to 0 and the provoking vertex is selected with the
       * PRIMITIVE.first_provoking_vertex field.
       */
      if (prim == MESA_PRIM_LINES) {
         /* The logic is inverted across arches. */
         cfg.flat_shading_vertex = rast->flatshade_first ^ (PAN_ARCH <= 5);
      }

      jm_emit_draw_descs(batch, &cfg, PIPE_SHADER_FRAGMENT);
#endif
   }
}

/* Packs a primitive descriptor, mostly common between Midgard/Bifrost tiler
 * jobs and Valhall IDVS jobs
 */
static void
jm_emit_primitive(struct panfrost_batch *batch,
                  const struct pipe_draw_info *info,
                  const struct pipe_draw_start_count_bias *draw,
                  bool secondary_shader, void *out)
{
   struct panfrost_context *ctx = batch->ctx;
   UNUSED struct pipe_rasterizer_state *rast = &ctx->rasterizer->base;

   pan_pack(out, PRIMITIVE, cfg) {
      cfg.draw_mode = pan_draw_mode(info->mode);
      if (panfrost_writes_point_size(ctx))
         cfg.point_size_array_format = MALI_POINT_SIZE_ARRAY_FORMAT_FP16;

#if PAN_ARCH <= 8
      /* For line primitives, PRIMITIVE.first_provoking_vertex must
       * be set to true and the provoking vertex is selected with
       * DRAW.flat_shading_vertex.
       */
      if (u_reduced_prim(info->mode) == MESA_PRIM_LINES)
         cfg.first_provoking_vertex = true;
      else
         cfg.first_provoking_vertex = rast->flatshade_first;

      if (panfrost_is_implicit_prim_restart(info)) {
         cfg.primitive_restart = MALI_PRIMITIVE_RESTART_IMPLICIT;
      } else if (info->primitive_restart) {
         cfg.primitive_restart = MALI_PRIMITIVE_RESTART_EXPLICIT;
         cfg.primitive_restart_index = info->restart_index;
      }

      cfg.job_task_split = 6;
#else
      struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];

      cfg.allow_rotating_primitives = allow_rotating_primitives(fs, info);
      cfg.primitive_restart = info->primitive_restart;

      /* Non-fixed restart indices should have been lowered */
      assert(!cfg.primitive_restart || panfrost_is_implicit_prim_restart(info));
#endif

      cfg.index_count = draw->count;
      cfg.index_type = panfrost_translate_index_size(info->index_size);

      if (PAN_ARCH >= 9) {
         /* Base vertex offset on Valhall is used for both
          * indexed and non-indexed draws, in a simple way for
          * either. Handle both cases.
          */
         if (cfg.index_type)
            cfg.base_vertex_offset = draw->index_bias;
         else
            cfg.base_vertex_offset = draw->start;

         /* Indices are moved outside the primitive descriptor
          * on Valhall, so we don't need to set that here
          */
      } else if (cfg.index_type) {
         cfg.base_vertex_offset = draw->index_bias - ctx->offset_start;

#if PAN_ARCH <= 7
         cfg.indices = batch->indices;
#endif
      }

#if PAN_ARCH >= 6
      cfg.secondary_shader = secondary_shader;
#endif
   }
}

#if PAN_ARCH == 9
static void
jm_emit_malloc_vertex_job(struct panfrost_batch *batch,
                          const struct pipe_draw_info *info,
                          const struct pipe_draw_start_count_bias *draw,
                          bool secondary_shader, void *job)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_compiled_shader *vs = ctx->prog[PIPE_SHADER_VERTEX];
   struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];

   bool fs_required = panfrost_fs_required(
      fs, ctx->blend, &ctx->pipe_framebuffer, ctx->depth_stencil);

   /* Varying shaders only feed data to the fragment shader, so if we omit
    * the fragment shader, we should omit the varying shader too.
    */
   secondary_shader &= fs_required;

   jm_emit_primitive(batch, info, draw, secondary_shader,
                     pan_section_ptr(job, MALLOC_VERTEX_JOB, PRIMITIVE));

   pan_section_pack(job, MALLOC_VERTEX_JOB, INSTANCE_COUNT, cfg) {
      cfg.count = info->instance_count;
   }

   pan_section_pack(job, MALLOC_VERTEX_JOB, ALLOCATION, cfg) {
      if (secondary_shader) {
         unsigned sz = panfrost_vertex_attribute_stride(vs, fs);
         cfg.vertex_packet_stride = sz + 16;
         cfg.vertex_attribute_stride = sz;
      } else {
         /* Hardware requirement for "no varyings" */
         cfg.vertex_packet_stride = 16;
         cfg.vertex_attribute_stride = 0;
      }
   }

   pan_section_pack(job, MALLOC_VERTEX_JOB, TILER, cfg) {
      cfg.address = jm_emit_tiler_desc(batch);
   }

   STATIC_ASSERT(sizeof(batch->scissor) == pan_size(SCISSOR));
   memcpy(pan_section_ptr(job, MALLOC_VERTEX_JOB, SCISSOR), &batch->scissor,
          pan_size(SCISSOR));

   panfrost_emit_primitive_size(
      ctx, info->mode == MESA_PRIM_POINTS, 0,
      pan_section_ptr(job, MALLOC_VERTEX_JOB, PRIMITIVE_SIZE));

   pan_section_pack(job, MALLOC_VERTEX_JOB, INDICES, cfg) {
      cfg.address = batch->indices;
   }

   jm_emit_tiler_draw(pan_section_ptr(job, MALLOC_VERTEX_JOB, DRAW), batch,
                      fs_required, u_reduced_prim(info->mode));

   pan_section_pack(job, MALLOC_VERTEX_JOB, POSITION, cfg) {
      jm_emit_shader_env(batch, &cfg, PIPE_SHADER_VERTEX,
                         panfrost_get_position_shader(batch, info));
   }

   pan_section_pack(job, MALLOC_VERTEX_JOB, VARYING, cfg) {
      /* If a varying shader is used, we configure it with the same
       * state as the position shader for backwards compatible
       * behaviour with Bifrost. This could be optimized.
       */
      if (!secondary_shader)
         continue;

      jm_emit_shader_env(batch, &cfg, PIPE_SHADER_VERTEX,
                         panfrost_get_varying_shader(batch));
   }
}
#endif

#if PAN_ARCH <= 7
static void
jm_emit_tiler_job(struct panfrost_batch *batch,
                  const struct pipe_draw_info *info,
                  const struct pipe_draw_start_count_bias *draw,
                  void *invocation_template, bool secondary_shader, void *job)
{
   struct panfrost_context *ctx = batch->ctx;

   void *section = pan_section_ptr(job, TILER_JOB, INVOCATION);
   memcpy(section, invocation_template, pan_size(INVOCATION));

   jm_emit_primitive(batch, info, draw, secondary_shader,
                     pan_section_ptr(job, TILER_JOB, PRIMITIVE));

   void *prim_size = pan_section_ptr(job, TILER_JOB, PRIMITIVE_SIZE);
   enum mesa_prim prim = u_reduced_prim(info->mode);

#if PAN_ARCH >= 6
   pan_section_pack(job, TILER_JOB, TILER, cfg) {
      cfg.address = jm_emit_tiler_desc(batch);
   }

   pan_section_pack(job, TILER_JOB, PADDING, cfg)
      ;
#endif

   jm_emit_tiler_draw(pan_section_ptr(job, TILER_JOB, DRAW), batch, true, prim);

   panfrost_emit_primitive_size(ctx, prim == MESA_PRIM_POINTS,
                                batch->varyings.psiz, prim_size);
}
#endif

void
GENX(jm_launch_xfb)(struct panfrost_batch *batch,
                    const struct pipe_draw_info *info, unsigned count)
{
   struct panfrost_ptr t = pan_pool_alloc_desc(&batch->pool.base, COMPUTE_JOB);

#if PAN_ARCH == 9
   pan_section_pack(t.cpu, COMPUTE_JOB, PAYLOAD, cfg) {
      cfg.workgroup_size_x = 1;
      cfg.workgroup_size_y = 1;
      cfg.workgroup_size_z = 1;

      cfg.workgroup_count_x = count;
      cfg.workgroup_count_y = info->instance_count;
      cfg.workgroup_count_z = 1;

      jm_emit_shader_env(batch, &cfg.compute, PIPE_SHADER_VERTEX,
                         batch->rsd[PIPE_SHADER_VERTEX]);

      /* TODO: Indexing. Also, this is a legacy feature... */
      cfg.compute.attribute_offset = batch->ctx->offset_start;

      /* Transform feedback shaders do not use barriers or shared
       * memory, so we may merge workgroups.
       */
      cfg.allow_merging_workgroups = true;
      cfg.task_increment = 1;
      cfg.task_axis = MALI_TASK_AXIS_Z;
   }
#else
   struct mali_invocation_packed invocation;

   panfrost_pack_work_groups_compute(&invocation, 1, count,
                                     info->instance_count, 1, 1, 1,
                                     PAN_ARCH <= 5, false);

   /* No varyings on XFB compute jobs. */
   mali_ptr saved_vs_varyings = batch->varyings.vs;

   batch->varyings.vs = 0;
   jm_emit_vertex_job(batch, info, &invocation, t.cpu);
   batch->varyings.vs = saved_vs_varyings;

#endif
   enum mali_job_type job_type = MALI_JOB_TYPE_COMPUTE;
#if PAN_ARCH <= 5
   job_type = MALI_JOB_TYPE_VERTEX;
#endif
   pan_jc_add_job(&batch->pool.base, &batch->jm.jobs.vtc_jc, job_type, true,
                  false, 0, 0, &t, false);
}

#if PAN_ARCH < 9
/*
 * Push jobs required for the rasterization pipeline. If there are side effects
 * from the vertex shader, these are handled ahead-of-time with a compute
 * shader. This function should not be called if rasterization is skipped.
 */
static void
jm_push_vertex_tiler_jobs(struct panfrost_batch *batch,
                          const struct panfrost_ptr *vertex_job,
                          const struct panfrost_ptr *tiler_job)
{
   unsigned vertex = pan_jc_add_job(&batch->pool.base, &batch->jm.jobs.vtc_jc,
                                    MALI_JOB_TYPE_VERTEX, false, false, 0, 0,
                                    vertex_job, false);

   pan_jc_add_job(&batch->pool.base, &batch->jm.jobs.vtc_jc,
                  MALI_JOB_TYPE_TILER, false, false, vertex, 0, tiler_job,
                  false);
}
#endif

void
GENX(jm_launch_draw)(struct panfrost_batch *batch,
                     const struct pipe_draw_info *info, unsigned drawid_offset,
                     const struct pipe_draw_start_count_bias *draw,
                     unsigned vertex_count)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_compiled_shader *vs = ctx->prog[PIPE_SHADER_VERTEX];
   bool secondary_shader = vs->info.vs.secondary_enable;
   bool idvs = vs->info.vs.idvs;

#if PAN_ARCH <= 7
   struct mali_invocation_packed invocation;
   if (info->instance_count > 1) {
      panfrost_pack_work_groups_compute(&invocation, 1, vertex_count,
                                        info->instance_count, 1, 1, 1, true,
                                        false);
   } else {
      pan_pack(&invocation, INVOCATION, cfg) {
         cfg.invocations = vertex_count - 1;
         cfg.size_y_shift = 0;
         cfg.size_z_shift = 0;
         cfg.workgroups_x_shift = 0;
         cfg.workgroups_y_shift = 0;
         cfg.workgroups_z_shift = 32;
         cfg.thread_group_split = MALI_SPLIT_MIN_EFFICIENT;
      }
   }

   /* Emit all sort of descriptors. */
#endif

   UNUSED struct panfrost_ptr tiler, vertex;

   if (idvs) {
#if PAN_ARCH == 9
      tiler = pan_pool_alloc_desc(&batch->pool.base, MALLOC_VERTEX_JOB);
#elif PAN_ARCH >= 6
      tiler = pan_pool_alloc_desc(&batch->pool.base, INDEXED_VERTEX_JOB);
#else
      unreachable("IDVS is unsupported on Midgard");
#endif
   } else {
      vertex = pan_pool_alloc_desc(&batch->pool.base, COMPUTE_JOB);
      tiler = pan_pool_alloc_desc(&batch->pool.base, TILER_JOB);
   }

#if PAN_ARCH == 9
   assert(idvs && "Memory allocated IDVS required on Valhall");

   jm_emit_malloc_vertex_job(batch, info, draw, secondary_shader, tiler.cpu);

   pan_jc_add_job(&batch->pool.base, &batch->jm.jobs.vtc_jc,
                  MALI_JOB_TYPE_MALLOC_VERTEX, false, false, 0, 0, &tiler,
                  false);
#else
   /* Fire off the draw itself */
   jm_emit_tiler_job(batch, info, draw, &invocation, secondary_shader,
                     tiler.cpu);
   if (idvs) {
#if PAN_ARCH >= 6
      jm_emit_vertex_draw(
         batch, pan_section_ptr(tiler.cpu, INDEXED_VERTEX_JOB, VERTEX_DRAW));

      pan_jc_add_job(&batch->pool.base, &batch->jm.jobs.vtc_jc,
                     MALI_JOB_TYPE_INDEXED_VERTEX, false, false, 0, 0, &tiler,
                     false);
#endif
   } else {
      jm_emit_vertex_job(batch, info, &invocation, vertex.cpu);
      jm_push_vertex_tiler_jobs(batch, &vertex, &tiler);
   }
#endif
}
