/*
 * Copyright © 2021 Collabora Ltd.
 *
 * Derived from tu_cmd_buffer.c which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * Copyright © 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "genxml/gen_macros.h"

#include "panvk_cs.h"
#include "panvk_private.h"

#include "pan_blitter.h"
#include "pan_desc.h"
#include "pan_encoder.h"

#include "util/rounding.h"
#include "util/u_pack_color.h"
#include "vk_format.h"

static uint32_t
panvk_debug_adjust_bo_flags(const struct panvk_device *device,
                            uint32_t bo_flags)
{
   uint32_t debug_flags = device->physical_device->instance->debug_flags;

   if (debug_flags & PANVK_DEBUG_DUMP)
      bo_flags &= ~PAN_BO_INVISIBLE;

   return bo_flags;
}

static void
panvk_cmd_prepare_fragment_job(struct panvk_cmd_buffer *cmdbuf)
{
   const struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;
   struct panvk_batch *batch = cmdbuf->state.batch;
   struct panfrost_ptr job_ptr =
      pan_pool_alloc_desc(&cmdbuf->desc_pool.base, FRAGMENT_JOB);

   GENX(pan_emit_fragment_job)
   (fbinfo, batch->fb.desc.gpu, job_ptr.cpu), batch->fragment_job = job_ptr.gpu;
   util_dynarray_append(&batch->jobs, void *, job_ptr.cpu);
}

void
panvk_per_arch(cmd_close_batch)(struct panvk_cmd_buffer *cmdbuf)
{
   struct panvk_batch *batch = cmdbuf->state.batch;

   if (!batch)
      return;

   const struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;

   assert(batch);

   bool clear = fbinfo->zs.clear.z | fbinfo->zs.clear.s;
   for (unsigned i = 0; i < fbinfo->rt_count; i++)
      clear |= fbinfo->rts[i].clear;

   if (!clear && !batch->jc.first_job) {
      if (util_dynarray_num_elements(&batch->event_ops,
                                     struct panvk_event_op) == 0) {
         /* Content-less batch, let's drop it */
         vk_free(&cmdbuf->vk.pool->alloc, batch);
      } else {
         /* Batch has no jobs but is needed for synchronization, let's add a
          * NULL job so the SUBMIT ioctl doesn't choke on it.
          */
         struct panfrost_ptr ptr =
            pan_pool_alloc_desc(&cmdbuf->desc_pool.base, JOB_HEADER);
         util_dynarray_append(&batch->jobs, void *, ptr.cpu);
         pan_jc_add_job(&cmdbuf->desc_pool.base, &batch->jc, MALI_JOB_TYPE_NULL,
                        false, false, 0, 0, &ptr, false);
         list_addtail(&batch->node, &cmdbuf->batches);
      }
      cmdbuf->state.batch = NULL;
      return;
   }

   struct panfrost_device *pdev = &cmdbuf->device->physical_device->pdev;

   list_addtail(&batch->node, &cmdbuf->batches);

   if (batch->jc.first_tiler) {
      struct panfrost_ptr preload_jobs[2];
      unsigned num_preload_jobs = GENX(pan_preload_fb)(
         &cmdbuf->desc_pool.base, &batch->jc, &cmdbuf->state.fb.info,
         batch->tls.gpu, batch->tiler.descs.gpu, preload_jobs);
      for (unsigned i = 0; i < num_preload_jobs; i++)
         util_dynarray_append(&batch->jobs, void *, preload_jobs[i].cpu);
   }

   if (batch->tlsinfo.tls.size) {
      unsigned size = panfrost_get_total_stack_size(
         batch->tlsinfo.tls.size, pdev->thread_tls_alloc, pdev->core_id_range);
      batch->tlsinfo.tls.ptr =
         pan_pool_alloc_aligned(&cmdbuf->tls_pool.base, size, 4096).gpu;
   }

   if (batch->tlsinfo.wls.size) {
      assert(batch->wls_total_size);
      batch->tlsinfo.wls.ptr =
         pan_pool_alloc_aligned(&cmdbuf->tls_pool.base, batch->wls_total_size,
                                4096)
            .gpu;
   }

   if (batch->tls.cpu)
      GENX(pan_emit_tls)(&batch->tlsinfo, batch->tls.cpu);

   if (batch->fb.desc.cpu) {
      batch->fb.desc.gpu |=
         GENX(pan_emit_fbd)(pdev, &cmdbuf->state.fb.info, &batch->tlsinfo,
                            &batch->tiler.ctx, batch->fb.desc.cpu);

      panvk_cmd_prepare_fragment_job(cmdbuf);
   }

   cmdbuf->state.batch = NULL;
}

void
panvk_per_arch(CmdNextSubpass2)(VkCommandBuffer commandBuffer,
                                const VkSubpassBeginInfo *pSubpassBeginInfo,
                                const VkSubpassEndInfo *pSubpassEndInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   cmdbuf->state.subpass++;
   panvk_cmd_fb_info_set_subpass(cmdbuf);
   panvk_cmd_open_batch(cmdbuf);
}

void
panvk_per_arch(CmdNextSubpass)(VkCommandBuffer cmd, VkSubpassContents contents)
{
   VkSubpassBeginInfo binfo = {.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO,
                               .contents = contents};
   VkSubpassEndInfo einfo = {
      .sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO,
   };

   panvk_per_arch(CmdNextSubpass2)(cmd, &binfo, &einfo);
}

void
panvk_per_arch(cmd_alloc_fb_desc)(struct panvk_cmd_buffer *cmdbuf)
{
   struct panvk_batch *batch = cmdbuf->state.batch;

   if (batch->fb.desc.gpu)
      return;

   const struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;
   bool has_zs_ext = fbinfo->zs.view.zs || fbinfo->zs.view.s;

   batch->fb.info = cmdbuf->state.framebuffer;
   batch->fb.desc = pan_pool_alloc_desc_aggregate(
      &cmdbuf->desc_pool.base, PAN_DESC(FRAMEBUFFER),
      PAN_DESC_ARRAY(has_zs_ext ? 1 : 0, ZS_CRC_EXTENSION),
      PAN_DESC_ARRAY(MAX2(fbinfo->rt_count, 1), RENDER_TARGET));

   memset(&cmdbuf->state.fb.info.bifrost.pre_post.dcds, 0,
          sizeof(cmdbuf->state.fb.info.bifrost.pre_post.dcds));
}

void
panvk_per_arch(cmd_alloc_tls_desc)(struct panvk_cmd_buffer *cmdbuf, bool gfx)
{
   struct panvk_batch *batch = cmdbuf->state.batch;

   assert(batch);
   if (!batch->tls.gpu) {
      batch->tls = pan_pool_alloc_desc(&cmdbuf->desc_pool.base, LOCAL_STORAGE);
   }
}

static void
panvk_cmd_prepare_draw_sysvals(
   struct panvk_cmd_buffer *cmdbuf,
   struct panvk_cmd_bind_point_state *bind_point_state,
   struct panvk_draw_info *draw)
{
   struct panvk_sysvals *sysvals = &bind_point_state->desc_state.sysvals;

   unsigned base_vertex = draw->index_size ? draw->vertex_offset : 0;
   if (sysvals->first_vertex != draw->offset_start ||
       sysvals->base_vertex != base_vertex ||
       sysvals->base_instance != draw->first_instance) {
      sysvals->first_vertex = draw->offset_start;
      sysvals->base_vertex = base_vertex;
      sysvals->base_instance = draw->first_instance;
      bind_point_state->desc_state.sysvals_ptr = 0;
   }

   if (cmdbuf->state.dirty & PANVK_DYNAMIC_BLEND_CONSTANTS) {
      memcpy(&sysvals->blend_constants, cmdbuf->state.blend.constants,
             sizeof(cmdbuf->state.blend.constants));
      bind_point_state->desc_state.sysvals_ptr = 0;
   }

   if (cmdbuf->state.dirty & PANVK_DYNAMIC_VIEWPORT) {
      panvk_sysval_upload_viewport_scale(&cmdbuf->state.viewport,
                                         &sysvals->viewport_scale);
      panvk_sysval_upload_viewport_offset(&cmdbuf->state.viewport,
                                          &sysvals->viewport_offset);
      bind_point_state->desc_state.sysvals_ptr = 0;
   }
}

static void
panvk_cmd_prepare_sysvals(struct panvk_cmd_buffer *cmdbuf,
                          struct panvk_cmd_bind_point_state *bind_point_state)
{
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;

   if (desc_state->sysvals_ptr)
      return;

   struct panfrost_ptr sysvals = pan_pool_alloc_aligned(
      &cmdbuf->desc_pool.base, sizeof(desc_state->sysvals), 16);
   memcpy(sysvals.cpu, &desc_state->sysvals, sizeof(desc_state->sysvals));
   desc_state->sysvals_ptr = sysvals.gpu;
}

static void
panvk_cmd_prepare_push_constants(
   struct panvk_cmd_buffer *cmdbuf,
   struct panvk_cmd_bind_point_state *bind_point_state)
{
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;

   if (!pipeline->layout->push_constants.size || desc_state->push_constants)
      return;

   struct panfrost_ptr push_constants = pan_pool_alloc_aligned(
      &cmdbuf->desc_pool.base,
      ALIGN_POT(pipeline->layout->push_constants.size, 16), 16);

   memcpy(push_constants.cpu, cmdbuf->push_constants,
          pipeline->layout->push_constants.size);
   desc_state->push_constants = push_constants.gpu;
}

static void
panvk_cmd_prepare_ubos(struct panvk_cmd_buffer *cmdbuf,
                       struct panvk_cmd_bind_point_state *bind_point_state)
{
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;

   if (!pipeline->num_ubos || desc_state->ubos)
      return;

   panvk_cmd_prepare_sysvals(cmdbuf, bind_point_state);
   panvk_cmd_prepare_push_constants(cmdbuf, bind_point_state);

   struct panfrost_ptr ubos = pan_pool_alloc_desc_array(
      &cmdbuf->desc_pool.base, pipeline->num_ubos, UNIFORM_BUFFER);

   panvk_per_arch(emit_ubos)(pipeline, desc_state, ubos.cpu);

   desc_state->ubos = ubos.gpu;
}

static void
panvk_cmd_prepare_textures(struct panvk_cmd_buffer *cmdbuf,
                           struct panvk_cmd_bind_point_state *bind_point_state)
{
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;
   unsigned num_textures = pipeline->layout->num_textures;

   if (!num_textures || desc_state->textures)
      return;

   struct panfrost_ptr textures = pan_pool_alloc_aligned(
      &cmdbuf->desc_pool.base, num_textures * pan_size(TEXTURE),
      pan_size(TEXTURE));

   void *texture = textures.cpu;

   for (unsigned i = 0; i < ARRAY_SIZE(desc_state->sets); i++) {
      if (!desc_state->sets[i])
         continue;

      memcpy(texture, desc_state->sets[i]->textures,
             desc_state->sets[i]->layout->num_textures * pan_size(TEXTURE));

      texture += desc_state->sets[i]->layout->num_textures * pan_size(TEXTURE);
   }

   desc_state->textures = textures.gpu;
}

static void
panvk_cmd_prepare_samplers(struct panvk_cmd_buffer *cmdbuf,
                           struct panvk_cmd_bind_point_state *bind_point_state)
{
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;
   unsigned num_samplers = pipeline->layout->num_samplers;

   if (!num_samplers || desc_state->samplers)
      return;

   struct panfrost_ptr samplers =
      pan_pool_alloc_desc_array(&cmdbuf->desc_pool.base, num_samplers, SAMPLER);

   void *sampler = samplers.cpu;

   /* Prepare the dummy sampler */
   pan_pack(sampler, SAMPLER, cfg) {
      cfg.seamless_cube_map = false;
      cfg.magnify_nearest = true;
      cfg.minify_nearest = true;
      cfg.normalized_coordinates = false;
   }

   sampler += pan_size(SAMPLER);

   for (unsigned i = 0; i < ARRAY_SIZE(desc_state->sets); i++) {
      if (!desc_state->sets[i])
         continue;

      memcpy(sampler, desc_state->sets[i]->samplers,
             desc_state->sets[i]->layout->num_samplers * pan_size(SAMPLER));

      sampler += desc_state->sets[i]->layout->num_samplers * pan_size(SAMPLER);
   }

   desc_state->samplers = samplers.gpu;
}

static void
panvk_draw_prepare_fs_rsd(struct panvk_cmd_buffer *cmdbuf,
                          struct panvk_draw_info *draw)
{
   const struct panvk_pipeline *pipeline =
      panvk_cmd_get_pipeline(cmdbuf, GRAPHICS);

   if (!pipeline->fs.dynamic_rsd) {
      draw->fs_rsd = pipeline->rsds[MESA_SHADER_FRAGMENT];
      return;
   }

   if (!cmdbuf->state.fs_rsd) {
      struct panfrost_ptr rsd = pan_pool_alloc_desc_aggregate(
         &cmdbuf->desc_pool.base, PAN_DESC(RENDERER_STATE),
         PAN_DESC_ARRAY(pipeline->blend.state.rt_count, BLEND));

      struct mali_renderer_state_packed rsd_dyn;
      struct mali_renderer_state_packed *rsd_templ =
         (struct mali_renderer_state_packed *)&pipeline->fs.rsd_template;

      STATIC_ASSERT(sizeof(pipeline->fs.rsd_template) >= sizeof(*rsd_templ));

      panvk_per_arch(emit_dyn_fs_rsd)(pipeline, &cmdbuf->state, &rsd_dyn);
      pan_merge(rsd_dyn, (*rsd_templ), RENDERER_STATE);
      memcpy(rsd.cpu, &rsd_dyn, sizeof(rsd_dyn));

      void *bd = rsd.cpu + pan_size(RENDERER_STATE);
      for (unsigned i = 0; i < pipeline->blend.state.rt_count; i++) {
         if (pipeline->blend.constant[i].index != (uint8_t)~0) {
            struct mali_blend_packed bd_dyn;
            struct mali_blend_packed *bd_templ =
               (struct mali_blend_packed *)&pipeline->blend.bd_template[i];

            STATIC_ASSERT(sizeof(pipeline->blend.bd_template[0]) >=
                          sizeof(*bd_templ));
            panvk_per_arch(emit_blend_constant)(cmdbuf->device, pipeline, i,
                                                cmdbuf->state.blend.constants,
                                                &bd_dyn);
            pan_merge(bd_dyn, (*bd_templ), BLEND);
            memcpy(bd, &bd_dyn, sizeof(bd_dyn));
         }
         bd += pan_size(BLEND);
      }

      cmdbuf->state.fs_rsd = rsd.gpu;
   }

   draw->fs_rsd = cmdbuf->state.fs_rsd;
}

void
panvk_per_arch(cmd_get_tiler_context)(struct panvk_cmd_buffer *cmdbuf,
                                      unsigned width, unsigned height)
{
   struct panvk_batch *batch = cmdbuf->state.batch;

   if (batch->tiler.descs.cpu)
      return;

   batch->tiler.descs = pan_pool_alloc_desc_aggregate(
      &cmdbuf->desc_pool.base, PAN_DESC(TILER_CONTEXT), PAN_DESC(TILER_HEAP));
   STATIC_ASSERT(sizeof(batch->tiler.templ) >=
                 pan_size(TILER_CONTEXT) + pan_size(TILER_HEAP));

   struct panfrost_ptr desc = {
      .gpu = batch->tiler.descs.gpu,
      .cpu = batch->tiler.templ,
   };

   panvk_per_arch(emit_tiler_context)(cmdbuf->device, width, height, &desc);
   memcpy(batch->tiler.descs.cpu, batch->tiler.templ,
          pan_size(TILER_CONTEXT) + pan_size(TILER_HEAP));
   batch->tiler.ctx.bifrost = batch->tiler.descs.gpu;
}

void
panvk_per_arch(cmd_prepare_tiler_context)(struct panvk_cmd_buffer *cmdbuf)
{
   const struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;

   panvk_per_arch(cmd_get_tiler_context)(cmdbuf, fbinfo->width, fbinfo->height);
}

static void
panvk_draw_prepare_tiler_context(struct panvk_cmd_buffer *cmdbuf,
                                 struct panvk_draw_info *draw)
{
   struct panvk_batch *batch = cmdbuf->state.batch;

   panvk_per_arch(cmd_prepare_tiler_context)(cmdbuf);
   draw->tiler_ctx = &batch->tiler.ctx;
}

static void
panvk_draw_prepare_varyings(struct panvk_cmd_buffer *cmdbuf,
                            struct panvk_draw_info *draw)
{
   const struct panvk_pipeline *pipeline =
      panvk_cmd_get_pipeline(cmdbuf, GRAPHICS);
   struct panvk_varyings_info *varyings = &cmdbuf->state.varyings;

   panvk_varyings_alloc(varyings, &cmdbuf->varying_pool.base,
                        draw->padded_vertex_count * draw->instance_count);

   unsigned buf_count = panvk_varyings_buf_count(varyings);
   struct panfrost_ptr bufs = pan_pool_alloc_desc_array(
      &cmdbuf->desc_pool.base, buf_count + 1, ATTRIBUTE_BUFFER);

   panvk_per_arch(emit_varying_bufs)(varyings, bufs.cpu);

   /* We need an empty entry to stop prefetching on Bifrost */
   memset(bufs.cpu + (pan_size(ATTRIBUTE_BUFFER) * buf_count), 0,
          pan_size(ATTRIBUTE_BUFFER));

   if (BITSET_TEST(varyings->active, VARYING_SLOT_POS)) {
      draw->position =
         varyings->buf[varyings->varying[VARYING_SLOT_POS].buf].address +
         varyings->varying[VARYING_SLOT_POS].offset;
   }

   if (pipeline->ia.writes_point_size) {
      draw->psiz =
         varyings->buf[varyings->varying[VARYING_SLOT_PSIZ].buf].address +
         varyings->varying[VARYING_SLOT_POS].offset;
   } else if (pipeline->ia.topology == MALI_DRAW_MODE_LINES ||
              pipeline->ia.topology == MALI_DRAW_MODE_LINE_STRIP ||
              pipeline->ia.topology == MALI_DRAW_MODE_LINE_LOOP) {
      draw->line_width = pipeline->dynamic_state_mask & PANVK_DYNAMIC_LINE_WIDTH
                            ? cmdbuf->state.rast.line_width
                            : pipeline->rast.line_width;
   } else {
      draw->line_width = 1.0f;
   }
   draw->varying_bufs = bufs.gpu;

   for (unsigned s = 0; s < MESA_SHADER_STAGES; s++) {
      if (!varyings->stage[s].count)
         continue;

      struct panfrost_ptr attribs = pan_pool_alloc_desc_array(
         &cmdbuf->desc_pool.base, varyings->stage[s].count, ATTRIBUTE);

      panvk_per_arch(emit_varyings)(cmdbuf->device, varyings, s, attribs.cpu);
      draw->stages[s].varyings = attribs.gpu;
   }
}

static void
panvk_fill_non_vs_attribs(struct panvk_cmd_buffer *cmdbuf,
                          struct panvk_cmd_bind_point_state *bind_point_state,
                          void *attrib_bufs, void *attribs, unsigned first_buf)
{
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;

   for (unsigned s = 0; s < pipeline->layout->vk.set_count; s++) {
      const struct panvk_descriptor_set *set = desc_state->sets[s];

      if (!set)
         continue;

      const struct panvk_descriptor_set_layout *layout = set->layout;
      unsigned img_idx = pipeline->layout->sets[s].img_offset;
      unsigned offset = img_idx * pan_size(ATTRIBUTE_BUFFER) * 2;
      unsigned size = layout->num_imgs * pan_size(ATTRIBUTE_BUFFER) * 2;

      memcpy(attrib_bufs + offset, desc_state->sets[s]->img_attrib_bufs, size);

      offset = img_idx * pan_size(ATTRIBUTE);
      for (unsigned i = 0; i < layout->num_imgs; i++) {
         pan_pack(attribs + offset, ATTRIBUTE, cfg) {
            cfg.buffer_index = first_buf + (img_idx + i) * 2;
            cfg.format = desc_state->sets[s]->img_fmts[i];
         }
         offset += pan_size(ATTRIBUTE);
      }
   }
}

static void
panvk_prepare_non_vs_attribs(struct panvk_cmd_buffer *cmdbuf,
                             struct panvk_cmd_bind_point_state *bind_point_state)
{
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;

   if (desc_state->non_vs_attribs || !pipeline->img_access_mask)
      return;

   unsigned attrib_count = pipeline->layout->num_imgs;
   unsigned attrib_buf_count = (pipeline->layout->num_imgs * 2);
   struct panfrost_ptr bufs = pan_pool_alloc_desc_array(
      &cmdbuf->desc_pool.base, attrib_buf_count + 1, ATTRIBUTE_BUFFER);
   struct panfrost_ptr attribs = pan_pool_alloc_desc_array(
      &cmdbuf->desc_pool.base, attrib_count, ATTRIBUTE);

   panvk_fill_non_vs_attribs(cmdbuf, bind_point_state, bufs.cpu, attribs.cpu,
                             0);

   desc_state->non_vs_attrib_bufs = bufs.gpu;
   desc_state->non_vs_attribs = attribs.gpu;
}

static void
panvk_draw_prepare_vs_attribs(struct panvk_cmd_buffer *cmdbuf,
                              struct panvk_draw_info *draw)
{
   struct panvk_cmd_bind_point_state *bind_point_state =
      panvk_cmd_get_bind_point_state(cmdbuf, GRAPHICS);
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;
   unsigned num_imgs =
      pipeline->img_access_mask & BITFIELD_BIT(MESA_SHADER_VERTEX)
         ? pipeline->layout->num_imgs
         : 0;
   unsigned attrib_count = pipeline->attribs.attrib_count + num_imgs;

   if (desc_state->vs_attribs || !attrib_count)
      return;

   if (!pipeline->attribs.buf_count) {
      panvk_prepare_non_vs_attribs(cmdbuf, bind_point_state);
      desc_state->vs_attrib_bufs = desc_state->non_vs_attrib_bufs;
      desc_state->vs_attribs = desc_state->non_vs_attribs;
      return;
   }

   unsigned attrib_buf_count = pipeline->attribs.buf_count * 2;
   struct panfrost_ptr bufs = pan_pool_alloc_desc_array(
      &cmdbuf->desc_pool.base, attrib_buf_count + 1, ATTRIBUTE_BUFFER);
   struct panfrost_ptr attribs = pan_pool_alloc_desc_array(
      &cmdbuf->desc_pool.base, attrib_count, ATTRIBUTE);

   panvk_per_arch(emit_attrib_bufs)(&pipeline->attribs, cmdbuf->state.vb.bufs,
                                    cmdbuf->state.vb.count, draw, bufs.cpu);
   panvk_per_arch(emit_attribs)(cmdbuf->device, draw, &pipeline->attribs,
                                cmdbuf->state.vb.bufs, cmdbuf->state.vb.count,
                                attribs.cpu);

   if (attrib_count > pipeline->attribs.buf_count) {
      unsigned bufs_offset =
         pipeline->attribs.buf_count * pan_size(ATTRIBUTE_BUFFER) * 2;
      unsigned attribs_offset =
         pipeline->attribs.buf_count * pan_size(ATTRIBUTE);

      panvk_fill_non_vs_attribs(
         cmdbuf, bind_point_state, bufs.cpu + bufs_offset,
         attribs.cpu + attribs_offset, pipeline->attribs.buf_count * 2);
   }

   /* A NULL entry is needed to stop prefecting on Bifrost */
   memset(bufs.cpu + (pan_size(ATTRIBUTE_BUFFER) * attrib_buf_count), 0,
          pan_size(ATTRIBUTE_BUFFER));

   desc_state->vs_attrib_bufs = bufs.gpu;
   desc_state->vs_attribs = attribs.gpu;
}

static void
panvk_draw_prepare_attributes(struct panvk_cmd_buffer *cmdbuf,
                              struct panvk_draw_info *draw)
{
   struct panvk_cmd_bind_point_state *bind_point_state =
      panvk_cmd_get_bind_point_state(cmdbuf, GRAPHICS);
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;

   for (unsigned i = 0; i < ARRAY_SIZE(draw->stages); i++) {
      if (i == MESA_SHADER_VERTEX) {
         panvk_draw_prepare_vs_attribs(cmdbuf, draw);
         draw->stages[i].attributes = desc_state->vs_attribs;
         draw->stages[i].attribute_bufs = desc_state->vs_attrib_bufs;
      } else if (pipeline->img_access_mask & BITFIELD_BIT(i)) {
         panvk_prepare_non_vs_attribs(cmdbuf, bind_point_state);
         draw->stages[i].attributes = desc_state->non_vs_attribs;
         draw->stages[i].attribute_bufs = desc_state->non_vs_attrib_bufs;
      }
   }
}

static void
panvk_draw_prepare_viewport(struct panvk_cmd_buffer *cmdbuf,
                            struct panvk_draw_info *draw)
{
   const struct panvk_pipeline *pipeline =
      panvk_cmd_get_pipeline(cmdbuf, GRAPHICS);

   if (pipeline->vpd) {
      draw->viewport = pipeline->vpd;
   } else if (cmdbuf->state.vpd) {
      draw->viewport = cmdbuf->state.vpd;
   } else {
      struct panfrost_ptr vp =
         pan_pool_alloc_desc(&cmdbuf->desc_pool.base, VIEWPORT);

      const VkViewport *viewport =
         pipeline->dynamic_state_mask & PANVK_DYNAMIC_VIEWPORT
            ? &cmdbuf->state.viewport
            : &pipeline->viewport;
      const VkRect2D *scissor =
         pipeline->dynamic_state_mask & PANVK_DYNAMIC_SCISSOR
            ? &cmdbuf->state.scissor
            : &pipeline->scissor;

      panvk_per_arch(emit_viewport)(viewport, scissor, vp.cpu);
      draw->viewport = cmdbuf->state.vpd = vp.gpu;
   }
}

static void
panvk_draw_prepare_vertex_job(struct panvk_cmd_buffer *cmdbuf,
                              struct panvk_draw_info *draw)
{
   const struct panvk_pipeline *pipeline =
      panvk_cmd_get_pipeline(cmdbuf, GRAPHICS);
   struct panvk_batch *batch = cmdbuf->state.batch;
   struct panfrost_ptr ptr =
      pan_pool_alloc_desc(&cmdbuf->desc_pool.base, COMPUTE_JOB);

   util_dynarray_append(&batch->jobs, void *, ptr.cpu);
   draw->jobs.vertex = ptr;
   panvk_per_arch(emit_vertex_job)(pipeline, draw, ptr.cpu);
}

static void
panvk_draw_prepare_tiler_job(struct panvk_cmd_buffer *cmdbuf,
                             struct panvk_draw_info *draw)
{
   const struct panvk_pipeline *pipeline =
      panvk_cmd_get_pipeline(cmdbuf, GRAPHICS);
   struct panvk_batch *batch = cmdbuf->state.batch;
   struct panfrost_ptr ptr =
      pan_pool_alloc_desc(&cmdbuf->desc_pool.base, TILER_JOB);

   util_dynarray_append(&batch->jobs, void *, ptr.cpu);
   draw->jobs.tiler = ptr;
   panvk_per_arch(emit_tiler_job)(pipeline, draw, ptr.cpu);
}

static void
panvk_cmd_draw(struct panvk_cmd_buffer *cmdbuf, struct panvk_draw_info *draw)
{
   struct panvk_batch *batch = cmdbuf->state.batch;
   struct panvk_cmd_bind_point_state *bind_point_state =
      panvk_cmd_get_bind_point_state(cmdbuf, GRAPHICS);
   const struct panvk_pipeline *pipeline =
      panvk_cmd_get_pipeline(cmdbuf, GRAPHICS);

   /* There are only 16 bits in the descriptor for the job ID, make sure all
    * the 3 (2 in Bifrost) jobs in this draw are in the same batch.
    */
   if (batch->jc.job_index >= (UINT16_MAX - 3)) {
      panvk_per_arch(cmd_close_batch)(cmdbuf);
      panvk_cmd_preload_fb_after_batch_split(cmdbuf);
      batch = panvk_cmd_open_batch(cmdbuf);
   }

   if (pipeline->rast.enable)
      panvk_per_arch(cmd_alloc_fb_desc)(cmdbuf);

   panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, true);

   panvk_cmd_prepare_draw_sysvals(cmdbuf, bind_point_state, draw);
   panvk_cmd_prepare_ubos(cmdbuf, bind_point_state);
   panvk_cmd_prepare_textures(cmdbuf, bind_point_state);
   panvk_cmd_prepare_samplers(cmdbuf, bind_point_state);

   /* TODO: indexed draws */
   struct panvk_descriptor_state *desc_state =
      panvk_cmd_get_desc_state(cmdbuf, GRAPHICS);

   draw->tls = batch->tls.gpu;
   draw->fb = batch->fb.desc.gpu;
   draw->ubos = desc_state->ubos;
   draw->textures = desc_state->textures;
   draw->samplers = desc_state->samplers;

   STATIC_ASSERT(sizeof(draw->invocation) >=
                 sizeof(struct mali_invocation_packed));
   panfrost_pack_work_groups_compute(
      (struct mali_invocation_packed *)&draw->invocation, 1, draw->vertex_range,
      draw->instance_count, 1, 1, 1, true, false);

   panvk_draw_prepare_fs_rsd(cmdbuf, draw);
   panvk_draw_prepare_varyings(cmdbuf, draw);
   panvk_draw_prepare_attributes(cmdbuf, draw);
   panvk_draw_prepare_viewport(cmdbuf, draw);
   panvk_draw_prepare_tiler_context(cmdbuf, draw);
   panvk_draw_prepare_vertex_job(cmdbuf, draw);
   panvk_draw_prepare_tiler_job(cmdbuf, draw);
   batch->tlsinfo.tls.size = MAX2(pipeline->tls_size, batch->tlsinfo.tls.size);
   assert(!pipeline->wls_size);

   unsigned vjob_id =
      pan_jc_add_job(&cmdbuf->desc_pool.base, &batch->jc, MALI_JOB_TYPE_VERTEX,
                     false, false, 0, 0, &draw->jobs.vertex, false);

   if (pipeline->rast.enable) {
      pan_jc_add_job(&cmdbuf->desc_pool.base, &batch->jc, MALI_JOB_TYPE_TILER,
                     false, false, vjob_id, 0, &draw->jobs.tiler, false);
   }

   /* Clear the dirty flags all at once */
   desc_state->dirty = cmdbuf->state.dirty = 0;
}

void
panvk_per_arch(CmdDraw)(VkCommandBuffer commandBuffer, uint32_t vertexCount,
                        uint32_t instanceCount, uint32_t firstVertex,
                        uint32_t firstInstance)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);

   if (instanceCount == 0 || vertexCount == 0)
      return;

   struct panvk_draw_info draw = {
      .first_vertex = firstVertex,
      .vertex_count = vertexCount,
      .vertex_range = vertexCount,
      .first_instance = firstInstance,
      .instance_count = instanceCount,
      .padded_vertex_count = instanceCount > 1
                                ? panfrost_padded_vertex_count(vertexCount)
                                : vertexCount,
      .offset_start = firstVertex,
   };

   panvk_cmd_draw(cmdbuf, &draw);
}

static void
panvk_index_minmax_search(struct panvk_cmd_buffer *cmdbuf, uint32_t start,
                          uint32_t count, bool restart, uint32_t *min,
                          uint32_t *max)
{
   void *ptr = cmdbuf->state.ib.buffer->bo->ptr.cpu +
               cmdbuf->state.ib.buffer->bo_offset + cmdbuf->state.ib.offset;

   assert(cmdbuf->state.ib.buffer);
   assert(cmdbuf->state.ib.buffer->bo);
   assert(cmdbuf->state.ib.buffer->bo->ptr.cpu);

   uint32_t debug_flags =
      cmdbuf->device->physical_device->instance->debug_flags;

   if (!(debug_flags & PANVK_DEBUG_NO_KNOWN_WARN)) {
      fprintf(
         stderr,
         "WARNING: Crawling index buffers from the CPU isn't valid in Vulkan\n");
   }

   *max = 0;

   /* TODO: Use panfrost_minmax_cache */
   /* TODO: Read full cacheline of data to mitigate the uncached
    * mapping slowness.
    */
   switch (cmdbuf->state.ib.index_size) {
#define MINMAX_SEARCH_CASE(sz)                                                 \
   case sz: {                                                                  \
      uint##sz##_t *indices = ptr;                                             \
      *min = UINT##sz##_MAX;                                                   \
      for (uint32_t i = 0; i < count; i++) {                                   \
         if (restart && indices[i + start] == UINT##sz##_MAX)                  \
            continue;                                                          \
         *min = MIN2(indices[i + start], *min);                                \
         *max = MAX2(indices[i + start], *max);                                \
      }                                                                        \
      break;                                                                   \
   }
      MINMAX_SEARCH_CASE(32)
      MINMAX_SEARCH_CASE(16)
      MINMAX_SEARCH_CASE(8)
#undef MINMAX_SEARCH_CASE
   default:
      unreachable("Invalid index size");
   }
}

void
panvk_per_arch(CmdDrawIndexed)(VkCommandBuffer commandBuffer,
                               uint32_t indexCount, uint32_t instanceCount,
                               uint32_t firstIndex, int32_t vertexOffset,
                               uint32_t firstInstance)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   uint32_t min_vertex, max_vertex;

   if (instanceCount == 0 || indexCount == 0)
      return;

   const struct panvk_pipeline *pipeline =
      panvk_cmd_get_pipeline(cmdbuf, GRAPHICS);
   bool primitive_restart = pipeline->ia.primitive_restart;

   panvk_index_minmax_search(cmdbuf, firstIndex, indexCount, primitive_restart,
                             &min_vertex, &max_vertex);

   unsigned vertex_range = max_vertex - min_vertex + 1;
   struct panvk_draw_info draw = {
      .index_size = cmdbuf->state.ib.index_size,
      .first_index = firstIndex,
      .index_count = indexCount,
      .vertex_offset = vertexOffset,
      .first_instance = firstInstance,
      .instance_count = instanceCount,
      .vertex_range = vertex_range,
      .vertex_count = indexCount + abs(vertexOffset),
      .padded_vertex_count = instanceCount > 1
                                ? panfrost_padded_vertex_count(vertex_range)
                                : vertex_range,
      .offset_start = min_vertex + vertexOffset,
      .indices = panvk_buffer_gpu_ptr(cmdbuf->state.ib.buffer,
                                      cmdbuf->state.ib.offset) +
                 (firstIndex * (cmdbuf->state.ib.index_size / 8)),
   };

   panvk_cmd_draw(cmdbuf, &draw);
}

VkResult
panvk_per_arch(EndCommandBuffer)(VkCommandBuffer commandBuffer)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   return vk_command_buffer_end(&cmdbuf->vk);
}

void
panvk_per_arch(CmdEndRenderPass2)(VkCommandBuffer commandBuffer,
                                  const VkSubpassEndInfo *pSubpassEndInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);

   panvk_per_arch(cmd_close_batch)(cmdbuf);
   vk_free(&cmdbuf->vk.pool->alloc, cmdbuf->state.clear);
   cmdbuf->state.batch = NULL;
   cmdbuf->state.pass = NULL;
   cmdbuf->state.subpass = NULL;
   cmdbuf->state.framebuffer = NULL;
   cmdbuf->state.clear = NULL;
}

void
panvk_per_arch(CmdEndRenderPass)(VkCommandBuffer cmd)
{
   VkSubpassEndInfo einfo = {
      .sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO,
   };

   panvk_per_arch(CmdEndRenderPass2)(cmd, &einfo);
}

void
panvk_per_arch(CmdPipelineBarrier2)(VkCommandBuffer commandBuffer,
                                    const VkDependencyInfo *pDependencyInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);

   /* Caches are flushed/invalidated at batch boundaries for now, nothing to do
    * for memory barriers assuming we implement barriers with the creation of a
    * new batch.
    * FIXME: We can probably do better with a CacheFlush job that has the
    * barrier flag set to true.
    */
   if (cmdbuf->state.batch) {
      panvk_per_arch(cmd_close_batch)(cmdbuf);
      panvk_cmd_preload_fb_after_batch_split(cmdbuf);
      panvk_cmd_open_batch(cmdbuf);
   }
}

static void
panvk_add_set_event_operation(struct panvk_cmd_buffer *cmdbuf,
                              struct panvk_event *event,
                              enum panvk_event_op_type type)
{
   struct panvk_event_op op = {
      .type = type,
      .event = event,
   };

   if (cmdbuf->state.batch == NULL) {
      /* No open batch, let's create a new one so this operation happens in
       * the right order.
       */
      panvk_cmd_open_batch(cmdbuf);
      util_dynarray_append(&cmdbuf->state.batch->event_ops,
                           struct panvk_event_op, op);
      panvk_per_arch(cmd_close_batch)(cmdbuf);
   } else {
      /* Let's close the current batch so the operation executes before any
       * future commands.
       */
      util_dynarray_append(&cmdbuf->state.batch->event_ops,
                           struct panvk_event_op, op);
      panvk_per_arch(cmd_close_batch)(cmdbuf);
      panvk_cmd_preload_fb_after_batch_split(cmdbuf);
      panvk_cmd_open_batch(cmdbuf);
   }
}

static void
panvk_add_wait_event_operation(struct panvk_cmd_buffer *cmdbuf,
                               struct panvk_event *event)
{
   struct panvk_event_op op = {
      .type = PANVK_EVENT_OP_WAIT,
      .event = event,
   };

   if (cmdbuf->state.batch == NULL) {
      /* No open batch, let's create a new one and have it wait for this event. */
      panvk_cmd_open_batch(cmdbuf);
      util_dynarray_append(&cmdbuf->state.batch->event_ops,
                           struct panvk_event_op, op);
   } else {
      /* Let's close the current batch so any future commands wait on the
       * event signal operation.
       */
      if (cmdbuf->state.batch->fragment_job ||
          cmdbuf->state.batch->jc.first_job) {
         panvk_per_arch(cmd_close_batch)(cmdbuf);
         panvk_cmd_preload_fb_after_batch_split(cmdbuf);
         panvk_cmd_open_batch(cmdbuf);
      }
      util_dynarray_append(&cmdbuf->state.batch->event_ops,
                           struct panvk_event_op, op);
   }
}

void
panvk_per_arch(CmdSetEvent2)(VkCommandBuffer commandBuffer, VkEvent _event,
                             const VkDependencyInfo *pDependencyInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_event, event, _event);

   /* vkCmdSetEvent cannot be called inside a render pass */
   assert(cmdbuf->state.pass == NULL);

   panvk_add_set_event_operation(cmdbuf, event, PANVK_EVENT_OP_SET);
}

void
panvk_per_arch(CmdResetEvent2)(VkCommandBuffer commandBuffer, VkEvent _event,
                               VkPipelineStageFlags2 stageMask)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_event, event, _event);

   /* vkCmdResetEvent cannot be called inside a render pass */
   assert(cmdbuf->state.pass == NULL);

   panvk_add_set_event_operation(cmdbuf, event, PANVK_EVENT_OP_RESET);
}

void
panvk_per_arch(CmdWaitEvents2)(VkCommandBuffer commandBuffer,
                               uint32_t eventCount, const VkEvent *pEvents,
                               const VkDependencyInfo *pDependencyInfos)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);

   assert(eventCount > 0);

   for (uint32_t i = 0; i < eventCount; i++) {
      VK_FROM_HANDLE(panvk_event, event, pEvents[i]);
      panvk_add_wait_event_operation(cmdbuf, event);
   }
}

static void
panvk_reset_cmdbuf(struct vk_command_buffer *vk_cmdbuf,
                   VkCommandBufferResetFlags flags)
{
   struct panvk_cmd_buffer *cmdbuf =
      container_of(vk_cmdbuf, struct panvk_cmd_buffer, vk);

   vk_command_buffer_reset(&cmdbuf->vk);

   list_for_each_entry_safe(struct panvk_batch, batch, &cmdbuf->batches, node) {
      list_del(&batch->node);
      util_dynarray_fini(&batch->jobs);
      util_dynarray_fini(&batch->event_ops);

      vk_free(&cmdbuf->vk.pool->alloc, batch);
   }

   panvk_pool_reset(&cmdbuf->desc_pool);
   panvk_pool_reset(&cmdbuf->tls_pool);
   panvk_pool_reset(&cmdbuf->varying_pool);

   for (unsigned i = 0; i < MAX_BIND_POINTS; i++)
      memset(&cmdbuf->bind_points[i].desc_state.sets, 0,
             sizeof(cmdbuf->bind_points[0].desc_state.sets));
}

static void
panvk_destroy_cmdbuf(struct vk_command_buffer *vk_cmdbuf)
{
   struct panvk_cmd_buffer *cmdbuf =
      container_of(vk_cmdbuf, struct panvk_cmd_buffer, vk);
   struct panvk_device *device = cmdbuf->device;

   list_for_each_entry_safe(struct panvk_batch, batch, &cmdbuf->batches, node) {
      list_del(&batch->node);
      util_dynarray_fini(&batch->jobs);
      util_dynarray_fini(&batch->event_ops);

      vk_free(&cmdbuf->vk.pool->alloc, batch);
   }

   panvk_pool_cleanup(&cmdbuf->desc_pool);
   panvk_pool_cleanup(&cmdbuf->tls_pool);
   panvk_pool_cleanup(&cmdbuf->varying_pool);
   vk_command_buffer_finish(&cmdbuf->vk);
   vk_free(&device->vk.alloc, cmdbuf);
}

static VkResult
panvk_create_cmdbuf(struct vk_command_pool *vk_pool,
                    struct vk_command_buffer **cmdbuf_out)
{
   struct panvk_device *device =
      container_of(vk_pool->base.device, struct panvk_device, vk);
   struct panvk_cmd_pool *pool =
      container_of(vk_pool, struct panvk_cmd_pool, vk);
   struct panvk_cmd_buffer *cmdbuf;

   cmdbuf = vk_zalloc(&device->vk.alloc, sizeof(*cmdbuf), 8,
                      VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmdbuf)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result = vk_command_buffer_init(&pool->vk, &cmdbuf->vk,
                                            &panvk_per_arch(cmd_buffer_ops), 0);
   if (result != VK_SUCCESS) {
      vk_free(&device->vk.alloc, cmdbuf);
      return result;
   }

   cmdbuf->device = device;

   panvk_pool_init(&cmdbuf->desc_pool, &device->physical_device->pdev,
                   &pool->desc_bo_pool, 0, 64 * 1024,
                   "Command buffer descriptor pool", true);
   panvk_pool_init(&cmdbuf->tls_pool, &device->physical_device->pdev,
                   &pool->tls_bo_pool,
                   panvk_debug_adjust_bo_flags(device, PAN_BO_INVISIBLE),
                   64 * 1024, "TLS pool", false);
   panvk_pool_init(&cmdbuf->varying_pool, &device->physical_device->pdev,
                   &pool->varying_bo_pool,
                   panvk_debug_adjust_bo_flags(device, PAN_BO_INVISIBLE),
                   64 * 1024, "Varyings pool", false);
   list_inithead(&cmdbuf->batches);
   *cmdbuf_out = &cmdbuf->vk;
   return VK_SUCCESS;
}

const struct vk_command_buffer_ops panvk_per_arch(cmd_buffer_ops) = {
   .create = panvk_create_cmdbuf,
   .reset = panvk_reset_cmdbuf,
   .destroy = panvk_destroy_cmdbuf,
};

VkResult
panvk_per_arch(BeginCommandBuffer)(VkCommandBuffer commandBuffer,
                                   const VkCommandBufferBeginInfo *pBeginInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);

   vk_command_buffer_begin(&cmdbuf->vk, pBeginInfo);

   memset(&cmdbuf->state, 0, sizeof(cmdbuf->state));

   return VK_SUCCESS;
}

void
panvk_per_arch(DestroyCommandPool)(VkDevice _device, VkCommandPool commandPool,
                                   const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_cmd_pool, pool, commandPool);

   vk_command_pool_finish(&pool->vk);

   panvk_bo_pool_cleanup(&pool->desc_bo_pool);
   panvk_bo_pool_cleanup(&pool->varying_bo_pool);
   panvk_bo_pool_cleanup(&pool->tls_bo_pool);

   vk_free2(&device->vk.alloc, pAllocator, pool);
}

void
panvk_per_arch(CmdDispatch)(VkCommandBuffer commandBuffer, uint32_t x,
                            uint32_t y, uint32_t z)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   const struct panfrost_device *pdev = &cmdbuf->device->physical_device->pdev;
   struct panvk_dispatch_info dispatch = {
      .wg_count = {x, y, z},
   };

   panvk_per_arch(cmd_close_batch)(cmdbuf);
   struct panvk_batch *batch = panvk_cmd_open_batch(cmdbuf);

   struct panvk_cmd_bind_point_state *bind_point_state =
      panvk_cmd_get_bind_point_state(cmdbuf, COMPUTE);
   struct panvk_descriptor_state *desc_state = &bind_point_state->desc_state;
   const struct panvk_pipeline *pipeline = bind_point_state->pipeline;
   struct panfrost_ptr job =
      pan_pool_alloc_desc(&cmdbuf->desc_pool.base, COMPUTE_JOB);

   struct panvk_sysvals *sysvals = &desc_state->sysvals;
   sysvals->num_work_groups.u32[0] = x;
   sysvals->num_work_groups.u32[1] = y;
   sysvals->num_work_groups.u32[2] = z;
   sysvals->local_group_size.u32[0] = pipeline->cs.local_size.x;
   sysvals->local_group_size.u32[1] = pipeline->cs.local_size.y;
   sysvals->local_group_size.u32[2] = pipeline->cs.local_size.z;
   desc_state->sysvals_ptr = 0;

   panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, false);
   dispatch.tsd = batch->tls.gpu;

   panvk_prepare_non_vs_attribs(cmdbuf, bind_point_state);
   dispatch.attributes = desc_state->non_vs_attribs;
   dispatch.attribute_bufs = desc_state->non_vs_attrib_bufs;

   panvk_cmd_prepare_ubos(cmdbuf, bind_point_state);
   dispatch.ubos = desc_state->ubos;

   panvk_cmd_prepare_textures(cmdbuf, bind_point_state);
   dispatch.textures = desc_state->textures;

   panvk_cmd_prepare_samplers(cmdbuf, bind_point_state);
   dispatch.samplers = desc_state->samplers;

   panvk_per_arch(emit_compute_job)(pipeline, &dispatch, job.cpu);
   pan_jc_add_job(&cmdbuf->desc_pool.base, &batch->jc, MALI_JOB_TYPE_COMPUTE,
                  false, false, 0, 0, &job, false);

   batch->tlsinfo.tls.size = pipeline->tls_size;
   batch->tlsinfo.wls.size = pipeline->wls_size;
   if (batch->tlsinfo.wls.size) {
      batch->wls_total_size =
         pan_wls_mem_size(pdev, &dispatch.wg_count, batch->tlsinfo.wls.size);
   }

   panvk_per_arch(cmd_close_batch)(cmdbuf);
   desc_state->dirty = 0;
}
