/*
 * Copyright © 2021 Collabora Ltd.
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

#include "nir/nir_builder.h"
#include "pan_blitter.h"
#include "pan_encoder.h"
#include "pan_shader.h"

#include "panvk_private.h"
#include "panvk_vX_meta.h"

#include "vk_format.h"

static mali_ptr
panvk_meta_clear_color_attachment_shader(struct panfrost_device *pdev,
                                         struct pan_pool *bin_pool,
                                         enum glsl_base_type base_type,
                                         struct pan_shader_info *shader_info)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_FRAGMENT, GENX(pan_shader_get_compiler_options)(),
      "panvk_meta_clear_attachment(base_type=%d)", base_type);

   const struct glsl_type *out_type = glsl_vector_type(base_type, 4);
   nir_variable *out =
      nir_variable_create(b.shader, nir_var_shader_out, out_type, "out");
   out->data.location = FRAG_RESULT_DATA0;

   nir_def *clear_values =
      nir_load_push_constant(&b, 4, 32, nir_imm_int(&b, 0), .range = ~0);
   nir_store_var(&b, out, clear_values, 0xff);

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(pdev),
      .is_blit = true,
      .no_ubo_to_push = true,
   };

   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);
   pan_shader_preprocess(b.shader, inputs.gpu_id);
   GENX(pan_shader_compile)(b.shader, &inputs, &binary, shader_info);

   shader_info->push.count = 4;

   mali_ptr shader =
      pan_pool_upload_aligned(bin_pool, binary.data, binary.size, 128);

   util_dynarray_fini(&binary);
   ralloc_free(b.shader);

   return shader;
}

static mali_ptr
panvk_meta_clear_color_attachment_emit_rsd(struct panfrost_device *pdev,
                                           struct pan_pool *desc_pool,
                                           enum pipe_format format, unsigned rt,
                                           struct pan_shader_info *shader_info,
                                           mali_ptr shader)
{
   struct panfrost_ptr rsd_ptr = pan_pool_alloc_desc_aggregate(
      desc_pool, PAN_DESC(RENDERER_STATE), PAN_DESC_ARRAY(rt + 1, BLEND));

   pan_pack(rsd_ptr.cpu, RENDERER_STATE, cfg) {
      pan_shader_prepare_rsd(shader_info, shader, &cfg);

      cfg.properties.depth_source = MALI_DEPTH_SOURCE_FIXED_FUNCTION;
      cfg.multisample_misc.sample_mask = UINT16_MAX;
      cfg.multisample_misc.depth_function = MALI_FUNC_ALWAYS;
      cfg.properties.allow_forward_pixel_to_be_killed = true;
      cfg.properties.allow_forward_pixel_to_kill = true;
      cfg.properties.zs_update_operation = MALI_PIXEL_KILL_WEAK_EARLY;
      cfg.properties.pixel_kill_operation = MALI_PIXEL_KILL_WEAK_EARLY;
   }

   void *bd = rsd_ptr.cpu + pan_size(RENDERER_STATE);

   pan_pack(bd, BLEND, cfg) {
      cfg.round_to_fb_precision = true;
      cfg.load_destination = false;
      cfg.equation.rgb.a = MALI_BLEND_OPERAND_A_SRC;
      cfg.equation.rgb.b = MALI_BLEND_OPERAND_B_SRC;
      cfg.equation.rgb.c = MALI_BLEND_OPERAND_C_ZERO;
      cfg.equation.alpha.a = MALI_BLEND_OPERAND_A_SRC;
      cfg.equation.alpha.b = MALI_BLEND_OPERAND_B_SRC;
      cfg.equation.alpha.c = MALI_BLEND_OPERAND_C_ZERO;
      cfg.internal.mode = MALI_BLEND_MODE_OPAQUE;
      cfg.equation.color_mask = 0xf;
      cfg.internal.fixed_function.num_comps = 4;
      cfg.internal.fixed_function.rt = rt;
      cfg.internal.fixed_function.conversion.memory_format =
         panfrost_format_to_bifrost_blend(pdev, format, false);
      cfg.internal.fixed_function.conversion.register_format =
         shader_info->bifrost.blend[0].format;
   }

   return rsd_ptr.gpu;
}

static mali_ptr
panvk_meta_clear_zs_attachment_emit_rsd(struct panfrost_device *pdev,
                                        struct pan_pool *desc_pool,
                                        VkImageAspectFlags mask,
                                        VkClearDepthStencilValue value)
{
   struct panfrost_ptr rsd_ptr = pan_pool_alloc_desc(desc_pool, RENDERER_STATE);

   pan_pack(rsd_ptr.cpu, RENDERER_STATE, cfg) {
      cfg.properties.depth_source = MALI_DEPTH_SOURCE_FIXED_FUNCTION;
      cfg.multisample_misc.sample_mask = UINT16_MAX;

      if (mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         cfg.multisample_misc.depth_write_mask = true;
         cfg.multisample_misc.depth_function = MALI_FUNC_NOT_EQUAL;

         if (value.depth != 0.0) {
            cfg.stencil_mask_misc.front_facing_depth_bias = true;
            cfg.stencil_mask_misc.back_facing_depth_bias = true;
            cfg.depth_units = INFINITY;
            cfg.depth_bias_clamp = value.depth;
         }
      }

      if (mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
         cfg.stencil_mask_misc.stencil_enable = true;
         cfg.stencil_mask_misc.stencil_mask_front = 0xFF;
         cfg.stencil_mask_misc.stencil_mask_back = 0xFF;

         cfg.stencil_front.compare_function = (mask & VK_IMAGE_ASPECT_DEPTH_BIT)
                                                 ? MALI_FUNC_ALWAYS
                                                 : MALI_FUNC_NOT_EQUAL;

         cfg.stencil_front.stencil_fail = MALI_STENCIL_OP_KEEP;
         cfg.stencil_front.depth_fail = MALI_STENCIL_OP_REPLACE;
         cfg.stencil_front.depth_pass = MALI_STENCIL_OP_REPLACE;
         cfg.stencil_front.reference_value = value.stencil;
         cfg.stencil_front.mask = 0xFF;
         cfg.stencil_back = cfg.stencil_front;
      }

      cfg.properties.allow_forward_pixel_to_be_killed = true;
      cfg.properties.zs_update_operation = MALI_PIXEL_KILL_WEAK_EARLY;
      cfg.properties.pixel_kill_operation = MALI_PIXEL_KILL_WEAK_EARLY;
   }

   return rsd_ptr.gpu;
}

static void
panvk_meta_clear_attachment_emit_dcd(struct pan_pool *pool, mali_ptr coords,
                                     mali_ptr push_constants, mali_ptr vpd,
                                     mali_ptr tsd, mali_ptr rsd, void *out)
{
   pan_pack(out, DRAW, cfg) {
      cfg.thread_storage = tsd;
      cfg.state = rsd;
      cfg.push_uniforms = push_constants;
      cfg.position = coords;
      cfg.viewport = vpd;
   }
}

static struct panfrost_ptr
panvk_meta_clear_attachment_emit_tiler_job(struct pan_pool *desc_pool,
                                           struct pan_jc *jc, mali_ptr coords,
                                           mali_ptr push_constants,
                                           mali_ptr vpd, mali_ptr rsd,
                                           mali_ptr tsd, mali_ptr tiler)
{
   struct panfrost_ptr job = pan_pool_alloc_desc(desc_pool, TILER_JOB);

   panvk_meta_clear_attachment_emit_dcd(
      desc_pool, coords, push_constants, vpd, tsd, rsd,
      pan_section_ptr(job.cpu, TILER_JOB, DRAW));

   pan_section_pack(job.cpu, TILER_JOB, PRIMITIVE, cfg) {
      cfg.draw_mode = MALI_DRAW_MODE_TRIANGLE_STRIP;
      cfg.index_count = 4;
      cfg.job_task_split = 6;
   }

   pan_section_pack(job.cpu, TILER_JOB, PRIMITIVE_SIZE, cfg) {
      cfg.constant = 1.0f;
   }

   void *invoc = pan_section_ptr(job.cpu, TILER_JOB, INVOCATION);
   panfrost_pack_work_groups_compute(invoc, 1, 4, 1, 1, 1, 1, true, false);

   pan_section_pack(job.cpu, TILER_JOB, PADDING, cfg)
      ;
   pan_section_pack(job.cpu, TILER_JOB, TILER, cfg) {
      cfg.address = tiler;
   }

   pan_jc_add_job(desc_pool, jc, MALI_JOB_TYPE_TILER, false, false, 0, 0, &job,
                  false);
   return job;
}

static enum glsl_base_type
panvk_meta_get_format_type(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   int i;

   i = util_format_get_first_non_void_channel(format);
   assert(i >= 0);

   if (desc->channel[i].normalized)
      return GLSL_TYPE_FLOAT;

   switch (desc->channel[i].type) {

   case UTIL_FORMAT_TYPE_UNSIGNED:
      return GLSL_TYPE_UINT;

   case UTIL_FORMAT_TYPE_SIGNED:
      return GLSL_TYPE_INT;

   case UTIL_FORMAT_TYPE_FLOAT:
      return GLSL_TYPE_FLOAT;

   default:
      unreachable("Unhandled format");
      return GLSL_TYPE_FLOAT;
   }
}

static void
panvk_meta_clear_attachment(struct panvk_cmd_buffer *cmdbuf,
                            unsigned attachment, unsigned rt,
                            VkImageAspectFlags mask,
                            const VkClearValue *clear_value,
                            const VkClearRect *clear_rect)
{
   struct panvk_physical_device *dev = cmdbuf->device->physical_device;
   struct panfrost_device *pdev = &dev->pdev;
   struct panvk_meta *meta = &cmdbuf->device->physical_device->meta;
   struct panvk_batch *batch = cmdbuf->state.batch;
   const struct panvk_render_pass *pass = cmdbuf->state.pass;
   const struct panvk_render_pass_attachment *att =
      &pass->attachments[attachment];
   unsigned minx = MAX2(clear_rect->rect.offset.x, 0);
   unsigned miny = MAX2(clear_rect->rect.offset.y, 0);
   unsigned maxx =
      MAX2(clear_rect->rect.offset.x + clear_rect->rect.extent.width - 1, 0);
   unsigned maxy =
      MAX2(clear_rect->rect.offset.y + clear_rect->rect.extent.height - 1, 0);

   panvk_per_arch(cmd_alloc_fb_desc)(cmdbuf);
   panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, true);
   panvk_per_arch(cmd_prepare_tiler_context)(cmdbuf);

   mali_ptr vpd = panvk_per_arch(meta_emit_viewport)(&cmdbuf->desc_pool.base,
                                                     minx, miny, maxx, maxy);

   float rect[] = {
      minx, miny,     0.0, 1.0, maxx + 1, miny,     0.0, 1.0,
      minx, maxy + 1, 0.0, 1.0, maxx + 1, maxy + 1, 0.0, 1.0,
   };
   mali_ptr coordinates =
      pan_pool_upload_aligned(&cmdbuf->desc_pool.base, rect, sizeof(rect), 64);

   enum glsl_base_type base_type = panvk_meta_get_format_type(att->format);

   mali_ptr tiler = batch->tiler.descs.gpu;
   mali_ptr tsd = batch->tls.gpu;

   mali_ptr pushconsts = 0, rsd = 0;

   if (mask & VK_IMAGE_ASPECT_COLOR_BIT) {
      mali_ptr shader = meta->clear_attachment.color[base_type].shader;
      struct pan_shader_info *shader_info =
         &meta->clear_attachment.color[base_type].shader_info;

      pushconsts = pan_pool_upload_aligned(&cmdbuf->desc_pool.base, clear_value,
                                           sizeof(*clear_value), 16);

      rsd = panvk_meta_clear_color_attachment_emit_rsd(
         pdev, &cmdbuf->desc_pool.base, att->format, rt, shader_info, shader);
   } else {
      rsd = panvk_meta_clear_zs_attachment_emit_rsd(
         pdev, &cmdbuf->desc_pool.base, mask, clear_value->depthStencil);
   }

   struct panfrost_ptr job;

   job = panvk_meta_clear_attachment_emit_tiler_job(
      &cmdbuf->desc_pool.base, &batch->jc, coordinates, pushconsts, vpd, rsd,
      tsd, tiler);

   util_dynarray_append(&batch->jobs, void *, job.cpu);
}

static void
panvk_meta_clear_color_img(struct panvk_cmd_buffer *cmdbuf,
                           struct panvk_image *img,
                           const VkClearColorValue *color,
                           const VkImageSubresourceRange *range)
{
   struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;
   struct pan_image_view view = {
      .format = img->pimage.layout.format,
      .dim = MALI_TEXTURE_DIMENSION_2D,
      .planes[0] = &img->pimage,
      .nr_samples = img->pimage.layout.nr_samples,
      .swizzle = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                  PIPE_SWIZZLE_W},
   };

   cmdbuf->state.fb.crc_valid[0] = false;
   *fbinfo = (struct pan_fb_info){
      .nr_samples = img->pimage.layout.nr_samples,
      .rt_count = 1,
      .rts[0].view = &view,
      .rts[0].clear = true,
      .rts[0].crc_valid = &cmdbuf->state.fb.crc_valid[0],
   };

   uint32_t clearval[4];
   pan_pack_color(panfrost_blendable_formats_v7, clearval,
                  (union pipe_color_union *)color, img->pimage.layout.format,
                  false);
   memcpy(fbinfo->rts[0].clear_value, clearval,
          sizeof(fbinfo->rts[0].clear_value));

   unsigned level_count = vk_image_subresource_level_count(&img->vk, range);
   unsigned layer_count = vk_image_subresource_layer_count(&img->vk, range);

   for (unsigned level = range->baseMipLevel;
        level < range->baseMipLevel + level_count; level++) {
      view.first_level = view.last_level = level;
      fbinfo->width = u_minify(img->pimage.layout.width, level);
      fbinfo->height = u_minify(img->pimage.layout.height, level);
      fbinfo->extent.maxx = fbinfo->width - 1;
      fbinfo->extent.maxy = fbinfo->height - 1;

      for (unsigned layer = range->baseArrayLayer;
           layer < range->baseArrayLayer + layer_count; layer++) {
         view.first_layer = view.last_layer = layer;
         panvk_cmd_open_batch(cmdbuf);
         panvk_per_arch(cmd_alloc_fb_desc)(cmdbuf);
         panvk_per_arch(cmd_close_batch)(cmdbuf);
      }
   }
}

void
panvk_per_arch(CmdClearColorImage)(VkCommandBuffer commandBuffer, VkImage image,
                                   VkImageLayout imageLayout,
                                   const VkClearColorValue *pColor,
                                   uint32_t rangeCount,
                                   const VkImageSubresourceRange *pRanges)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_image, img, image);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   for (unsigned i = 0; i < rangeCount; i++)
      panvk_meta_clear_color_img(cmdbuf, img, pColor, &pRanges[i]);
}

static void
panvk_meta_clear_zs_img(struct panvk_cmd_buffer *cmdbuf,
                        struct panvk_image *img,
                        const VkClearDepthStencilValue *value,
                        const VkImageSubresourceRange *range)
{
   struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;
   struct pan_image_view view = {
      .format = img->pimage.layout.format,
      .dim = MALI_TEXTURE_DIMENSION_2D,
      .planes[0] = &img->pimage,
      .nr_samples = img->pimage.layout.nr_samples,
      .swizzle = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                  PIPE_SWIZZLE_W},
   };

   cmdbuf->state.fb.crc_valid[0] = false;
   *fbinfo = (struct pan_fb_info){
      .nr_samples = img->pimage.layout.nr_samples,
      .rt_count = 1,
      .zs.clear_value.depth = value->depth,
      .zs.clear_value.stencil = value->stencil,
      .zs.clear.z = range->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT,
      .zs.clear.s = range->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT};

   const struct util_format_description *fdesc =
      util_format_description(view.format);

   if (util_format_has_depth(fdesc)) {
      fbinfo->zs.view.zs = &view;
      if (util_format_has_stencil(fdesc)) {
         fbinfo->zs.preload.z = !fbinfo->zs.clear.z;
         fbinfo->zs.preload.s = !fbinfo->zs.clear.s;
      }
   } else {
      fbinfo->zs.view.s = &view;
   }

   unsigned level_count = vk_image_subresource_level_count(&img->vk, range);
   unsigned layer_count = vk_image_subresource_layer_count(&img->vk, range);

   for (unsigned level = range->baseMipLevel;
        level < range->baseMipLevel + level_count; level++) {
      view.first_level = view.last_level = level;
      fbinfo->width = u_minify(img->pimage.layout.width, level);
      fbinfo->height = u_minify(img->pimage.layout.height, level);
      fbinfo->extent.maxx = fbinfo->width - 1;
      fbinfo->extent.maxy = fbinfo->height - 1;

      for (unsigned layer = range->baseArrayLayer;
           layer < range->baseArrayLayer + layer_count; layer++) {
         view.first_layer = view.last_layer = layer;
         panvk_cmd_open_batch(cmdbuf);
         panvk_per_arch(cmd_alloc_fb_desc)(cmdbuf);
         panvk_per_arch(cmd_close_batch)(cmdbuf);
      }
   }

   memset(fbinfo, 0, sizeof(*fbinfo));
}

void
panvk_per_arch(CmdClearDepthStencilImage)(
   VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
   const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
   const VkImageSubresourceRange *pRanges)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_image, img, image);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   for (unsigned i = 0; i < rangeCount; i++)
      panvk_meta_clear_zs_img(cmdbuf, img, pDepthStencil, &pRanges[i]);
}

void
panvk_per_arch(CmdClearAttachments)(VkCommandBuffer commandBuffer,
                                    uint32_t attachmentCount,
                                    const VkClearAttachment *pAttachments,
                                    uint32_t rectCount,
                                    const VkClearRect *pRects)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   const struct panvk_subpass *subpass = cmdbuf->state.subpass;

   for (unsigned i = 0; i < attachmentCount; i++) {
      for (unsigned j = 0; j < rectCount; j++) {

         uint32_t attachment, rt = 0;
         if (pAttachments[i].aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
            rt = pAttachments[i].colorAttachment;
            attachment = subpass->color_attachments[rt].idx;
         } else {
            attachment = subpass->zs_attachment.idx;
         }

         if (attachment == VK_ATTACHMENT_UNUSED)
            continue;

         panvk_meta_clear_attachment(cmdbuf, attachment, rt,
                                     pAttachments[i].aspectMask,
                                     &pAttachments[i].clearValue, &pRects[j]);
      }
   }
}

static void
panvk_meta_clear_attachment_init(struct panvk_physical_device *dev)
{
   dev->meta.clear_attachment.color[GLSL_TYPE_UINT].shader =
      panvk_meta_clear_color_attachment_shader(
         &dev->pdev, &dev->meta.bin_pool.base, GLSL_TYPE_UINT,
         &dev->meta.clear_attachment.color[GLSL_TYPE_UINT].shader_info);

   dev->meta.clear_attachment.color[GLSL_TYPE_INT].shader =
      panvk_meta_clear_color_attachment_shader(
         &dev->pdev, &dev->meta.bin_pool.base, GLSL_TYPE_INT,
         &dev->meta.clear_attachment.color[GLSL_TYPE_INT].shader_info);

   dev->meta.clear_attachment.color[GLSL_TYPE_FLOAT].shader =
      panvk_meta_clear_color_attachment_shader(
         &dev->pdev, &dev->meta.bin_pool.base, GLSL_TYPE_FLOAT,
         &dev->meta.clear_attachment.color[GLSL_TYPE_FLOAT].shader_info);
}

void
panvk_per_arch(meta_clear_init)(struct panvk_physical_device *dev)
{
   panvk_meta_clear_attachment_init(dev);
}
