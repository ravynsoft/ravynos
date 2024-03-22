/*
 * Copyright © 2021 Raspberry Pi Ltd
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
 */

#include "v3dv_private.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/common/v3d_util.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/compiler/v3d_compiler.h"

#include "util/half_float.h"
#include "vulkan/util/vk_format.h"
#include "util/u_pack_color.h"

void
v3dX(job_emit_binning_flush)(struct v3dv_job *job)
{
   assert(job);

   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(FLUSH));
   v3dv_return_if_oom(NULL, job);

   cl_emit(&job->bcl, FLUSH, flush);
}

void
v3dX(job_emit_enable_double_buffer)(struct v3dv_job *job)
{
   assert(job->can_use_double_buffer);
   assert(job->frame_tiling.double_buffer);
   assert(!job->frame_tiling.msaa);
   assert(job->bcl_tile_binning_mode_ptr);

   const struct v3dv_frame_tiling *tiling = &job->frame_tiling;
   struct cl_packet_struct(TILE_BINNING_MODE_CFG) config = {
      cl_packet_header(TILE_BINNING_MODE_CFG),
   };
   config.width_in_pixels = tiling->width;
   config.height_in_pixels = tiling->height;
#if V3D_VERSION == 42
   config.number_of_render_targets = MAX2(tiling->render_target_count, 1);
   config.multisample_mode_4x = tiling->msaa;
   config.double_buffer_in_non_ms_mode = tiling->double_buffer;
   config.maximum_bpp_of_all_render_targets = tiling->internal_bpp;
#endif
#if V3D_VERSION >= 71
      unreachable("HW generation 71 not supported yet.");
#endif

   uint8_t *rewrite_addr = (uint8_t *)job->bcl_tile_binning_mode_ptr;
   cl_packet_pack(TILE_BINNING_MODE_CFG)(NULL, rewrite_addr, &config);
}

void
v3dX(job_emit_binning_prolog)(struct v3dv_job *job,
                              const struct v3dv_frame_tiling *tiling,
                              uint32_t layers)
{
   /* This must go before the binning mode configuration. It is
    * required for layered framebuffers to work.
    */
   cl_emit(&job->bcl, NUMBER_OF_LAYERS, config) {
      config.number_of_layers = layers;
   }

   assert(!tiling->double_buffer || !tiling->msaa);
   job->bcl_tile_binning_mode_ptr = cl_start(&job->bcl);
   cl_emit(&job->bcl, TILE_BINNING_MODE_CFG, config) {
      config.width_in_pixels = tiling->width;
      config.height_in_pixels = tiling->height;
#if V3D_VERSION == 42
      config.number_of_render_targets = MAX2(tiling->render_target_count, 1);
      config.multisample_mode_4x = tiling->msaa;
      config.double_buffer_in_non_ms_mode = tiling->double_buffer;
      config.maximum_bpp_of_all_render_targets = tiling->internal_bpp;
#endif
#if V3D_VERSION >= 71
      config.log2_tile_width = log2_tile_size(tiling->tile_width);
      config.log2_tile_height = log2_tile_size(tiling->tile_height);
      /* FIXME: ideally we would like next assert on the packet header (as is
       * general, so also applies to GL). We would need to expand
       * gen_pack_header for that.
       */
      assert(config.log2_tile_width == config.log2_tile_height ||
             config.log2_tile_width == config.log2_tile_height + 1);
#endif
   }

   /* There's definitely nothing in the VCD cache we want. */
   cl_emit(&job->bcl, FLUSH_VCD_CACHE, bin);

   /* "Binning mode lists must have a Start Tile Binning item (6) after
    *  any prefix state data before the binning list proper starts."
    */
   cl_emit(&job->bcl, START_TILE_BINNING, bin);
}

void
v3dX(cmd_buffer_end_render_pass_secondary)(struct v3dv_cmd_buffer *cmd_buffer)
{
   assert(cmd_buffer->state.job);
   v3dv_cl_ensure_space_with_branch(&cmd_buffer->state.job->bcl,
                                    cl_packet_length(RETURN_FROM_SUB_LIST));
   v3dv_return_if_oom(cmd_buffer, NULL);
   cl_emit(&cmd_buffer->state.job->bcl, RETURN_FROM_SUB_LIST, ret);
}

void
v3dX(job_emit_clip_window)(struct v3dv_job *job, const VkRect2D *rect)
{
   assert(job);

   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(CLIP_WINDOW));
   v3dv_return_if_oom(NULL, job);

   cl_emit(&job->bcl, CLIP_WINDOW, clip) {
      clip.clip_window_left_pixel_coordinate = rect->offset.x;
      clip.clip_window_bottom_pixel_coordinate = rect->offset.y;
      clip.clip_window_width_in_pixels = rect->extent.width;
      clip.clip_window_height_in_pixels = rect->extent.height;
   }
}

static void
cmd_buffer_render_pass_emit_load(struct v3dv_cmd_buffer *cmd_buffer,
                                 struct v3dv_cl *cl,
                                 struct v3dv_image_view *iview,
                                 uint32_t layer,
                                 uint32_t buffer)
{
   const struct v3dv_image *image = (struct v3dv_image *) iview->vk.image;

   /* We don't support rendering to ycbcr images, so the image view should be
    * single-plane, and using a single-plane format. But note that the underlying
    * image can be a ycbcr format, as we support rendering to a specific plane
    * of an image. This is used for example on some meta_copy code paths, in
    * order to copy from/to a plane of a ycbcr image.
    */
   assert(iview->plane_count == 1);
   assert(iview->format->plane_count == 1);

   uint8_t image_plane = v3dv_plane_from_aspect(iview->vk.aspects);
   const struct v3d_resource_slice *slice =
      &image->planes[image_plane].slices[iview->vk.base_mip_level];

   uint32_t layer_offset =
      v3dv_layer_offset(image, iview->vk.base_mip_level,
                        iview->vk.base_array_layer + layer, image_plane);

   cl_emit(cl, LOAD_TILE_BUFFER_GENERAL, load) {
      load.buffer_to_load = buffer;
      load.address = v3dv_cl_address(image->planes[image_plane].mem->bo, layer_offset);

      load.input_image_format = iview->format->planes[0].rt_type;

      /* If we create an image view with only the stencil format, we
       * re-interpret the format as RGBA8_UINT, as it is want we want in
       * general (see CreateImageView).
       *
       * However, when we are loading/storing tiles from the ZSTENCIL tile
       * buffer, we need to use the underlying DS format.
       */
      if (buffer == ZSTENCIL &&
          iview->format->planes[0].rt_type == V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI) {
         assert(image->format->planes[image_plane].rt_type == V3D_OUTPUT_IMAGE_FORMAT_D24S8);
         load.input_image_format = image->format->planes[image_plane].rt_type;
      }

      load.r_b_swap = iview->planes[0].swap_rb;
      load.channel_reverse = iview->planes[0].channel_reverse;
      load.memory_format = slice->tiling;

      if (slice->tiling == V3D_TILING_UIF_NO_XOR ||
          slice->tiling == V3D_TILING_UIF_XOR) {
         load.height_in_ub_or_stride =
            slice->padded_height_of_output_image_in_uif_blocks;
      } else if (slice->tiling == V3D_TILING_RASTER) {
         load.height_in_ub_or_stride = slice->stride;
      }

      if (image->vk.samples > VK_SAMPLE_COUNT_1_BIT)
         load.decimate_mode = V3D_DECIMATE_MODE_ALL_SAMPLES;
      else
         load.decimate_mode = V3D_DECIMATE_MODE_SAMPLE_0;
   }
}

static inline uint32_t
v3dv_zs_buffer(bool depth, bool stencil)
{
   if (depth && stencil)
      return ZSTENCIL;
   else if (depth)
      return Z;
   else if (stencil)
      return STENCIL;
   return NONE;
}

static void
cmd_buffer_render_pass_emit_loads(struct v3dv_cmd_buffer *cmd_buffer,
                                  struct v3dv_cl *cl,
                                  uint32_t layer)
{
   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   const struct v3dv_render_pass *pass = state->pass;
   const struct v3dv_subpass *subpass = &pass->subpasses[state->subpass_idx];

  assert(!pass->multiview_enabled || layer < MAX_MULTIVIEW_VIEW_COUNT);

   for (uint32_t i = 0; i < subpass->color_count; i++) {
      uint32_t attachment_idx = subpass->color_attachments[i].attachment;

      if (attachment_idx == VK_ATTACHMENT_UNUSED)
         continue;

      const struct v3dv_render_pass_attachment *attachment =
         &state->pass->attachments[attachment_idx];

      /* According to the Vulkan spec:
       *
       *    "The load operation for each sample in an attachment happens before
       *     any recorded command which accesses the sample in the first subpass
       *     where the attachment is used."
       *
       * If the load operation is CLEAR, we must only clear once on the first
       * subpass that uses the attachment (and in that case we don't LOAD).
       * After that, we always want to load so we don't lose any rendering done
       * by a previous subpass to the same attachment. We also want to load
       * if the current job is continuing subpass work started by a previous
       * job, for the same reason.
       *
       * If the render area is not aligned to tile boundaries then we have
       * tiles which are partially covered by it. In this case, we need to
       * load the tiles so we can preserve the pixels that are outside the
       * render area for any such tiles.
       */
      uint32_t first_subpass = !pass->multiview_enabled ?
         attachment->first_subpass :
         attachment->views[layer].first_subpass;

      uint32_t last_subpass = !pass->multiview_enabled ?
         attachment->last_subpass :
         attachment->views[layer].last_subpass;

      bool needs_load =
         v3dv_cmd_buffer_check_needs_load(state,
                                          VK_IMAGE_ASPECT_COLOR_BIT,
                                          first_subpass,
                                          attachment->desc.loadOp,
                                          last_subpass,
                                          attachment->desc.storeOp);
      if (needs_load) {
         struct v3dv_image_view *iview =
            state->attachments[attachment_idx].image_view;
         cmd_buffer_render_pass_emit_load(cmd_buffer, cl, iview,
                                          layer, RENDER_TARGET_0 + i);
      }
   }

   uint32_t ds_attachment_idx = subpass->ds_attachment.attachment;
   if (ds_attachment_idx != VK_ATTACHMENT_UNUSED) {
      const struct v3dv_render_pass_attachment *ds_attachment =
         &state->pass->attachments[ds_attachment_idx];

      const VkImageAspectFlags ds_aspects =
         vk_format_aspects(ds_attachment->desc.format);

      uint32_t ds_first_subpass = !pass->multiview_enabled ?
         ds_attachment->first_subpass :
         ds_attachment->views[layer].first_subpass;

      uint32_t ds_last_subpass = !pass->multiview_enabled ?
         ds_attachment->last_subpass :
         ds_attachment->views[layer].last_subpass;

      const bool needs_depth_load =
         v3dv_cmd_buffer_check_needs_load(state,
                                          ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                          ds_first_subpass,
                                          ds_attachment->desc.loadOp,
                                          ds_last_subpass,
                                          ds_attachment->desc.storeOp);

      const bool needs_stencil_load =
         v3dv_cmd_buffer_check_needs_load(state,
                                          ds_aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                                          ds_first_subpass,
                                          ds_attachment->desc.stencilLoadOp,
                                          ds_last_subpass,
                                          ds_attachment->desc.stencilStoreOp);

      if (needs_depth_load || needs_stencil_load) {
         struct v3dv_image_view *iview =
            state->attachments[ds_attachment_idx].image_view;
         /* From the Vulkan spec:
          *
          *   "When an image view of a depth/stencil image is used as a
          *   depth/stencil framebuffer attachment, the aspectMask is ignored
          *   and both depth and stencil image subresources are used."
          *
          * So we ignore the aspects from the subresource range of the image
          * view for the depth/stencil attachment, but we still need to restrict
          * the to aspects compatible with the render pass and the image.
          */
         const uint32_t zs_buffer =
            v3dv_zs_buffer(needs_depth_load, needs_stencil_load);
         cmd_buffer_render_pass_emit_load(cmd_buffer, cl,
                                          iview, layer, zs_buffer);
      }
   }

   cl_emit(cl, END_OF_LOADS, end);
}

static void
cmd_buffer_render_pass_emit_store(struct v3dv_cmd_buffer *cmd_buffer,
                                  struct v3dv_cl *cl,
                                  uint32_t attachment_idx,
                                  uint32_t layer,
                                  uint32_t buffer,
                                  bool clear,
                                  bool is_multisample_resolve)
{
   const struct v3dv_image_view *iview =
      cmd_buffer->state.attachments[attachment_idx].image_view;
   const struct v3dv_image *image = (struct v3dv_image *) iview->vk.image;

   /* We don't support rendering to ycbcr images, so the image view should be
    * one-plane, and using a single-plane format. But note that the underlying
    * image can be a ycbcr format, as we support rendering to a specific plane
    * of an image. This is used for example on some meta_copy code paths, in
    * order to copy from/to a plane of a ycbcr image.
    */
   assert(iview->plane_count == 1);
   assert(iview->format->plane_count == 1);

   uint8_t image_plane = v3dv_plane_from_aspect(iview->vk.aspects);
   const struct v3d_resource_slice *slice =
      &image->planes[image_plane].slices[iview->vk.base_mip_level];
   uint32_t layer_offset = v3dv_layer_offset(image,
                                             iview->vk.base_mip_level,
                                             iview->vk.base_array_layer + layer,
                                             image_plane);

   /* The Clear Buffer bit is not supported for Z/Stencil stores in 7.x and it
    * is broken in earlier V3D versions.
    */
   assert((buffer != Z && buffer != STENCIL && buffer != ZSTENCIL) || !clear);

   cl_emit(cl, STORE_TILE_BUFFER_GENERAL, store) {
      store.buffer_to_store = buffer;
      store.address = v3dv_cl_address(image->planes[image_plane].mem->bo, layer_offset);
      store.clear_buffer_being_stored = clear;

      store.output_image_format = iview->format->planes[0].rt_type;

      /* If we create an image view with only the stencil format, we
       * re-interpret the format as RGBA8_UINT, as it is want we want in
       * general (see CreateImageView).
       *
       * However, when we are loading/storing tiles from the ZSTENCIL tile
       * buffer, we need to use the underlying DS format.
       */
      if (buffer == ZSTENCIL &&
          iview->format->planes[0].rt_type == V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI) {
         assert(image->format->planes[image_plane].rt_type == V3D_OUTPUT_IMAGE_FORMAT_D24S8);
         store.output_image_format = image->format->planes[image_plane].rt_type;
      }

      store.r_b_swap = iview->planes[0].swap_rb;
      store.channel_reverse = iview->planes[0].channel_reverse;
      store.memory_format = slice->tiling;

      if (slice->tiling == V3D_TILING_UIF_NO_XOR ||
          slice->tiling == V3D_TILING_UIF_XOR) {
         store.height_in_ub_or_stride =
            slice->padded_height_of_output_image_in_uif_blocks;
      } else if (slice->tiling == V3D_TILING_RASTER) {
         store.height_in_ub_or_stride = slice->stride;
      }

      if (image->vk.samples > VK_SAMPLE_COUNT_1_BIT)
         store.decimate_mode = V3D_DECIMATE_MODE_ALL_SAMPLES;
      else if (is_multisample_resolve)
         store.decimate_mode = V3D_DECIMATE_MODE_4X;
      else
         store.decimate_mode = V3D_DECIMATE_MODE_SAMPLE_0;
   }
}

static bool
check_needs_clear(const struct v3dv_cmd_buffer_state *state,
                  VkImageAspectFlags aspect,
                  uint32_t first_subpass_idx,
                  VkAttachmentLoadOp load_op,
                  bool do_clear_with_draw)
{
   /* We call this with image->vk.aspects & aspect, so 0 means the aspect we are
    * testing does not exist in the image.
    */
   if (!aspect)
      return false;

   /* If the aspect needs to be cleared with a draw call then we won't emit
    * the clear here.
    */
   if (do_clear_with_draw)
      return false;

   /* If this is resuming a subpass started with another job, then attachment
    * load operations don't apply.
    */
   if (state->job->is_subpass_continue)
      return false;

   /* If the render area is not aligned to tile boundaries we can't use the
    * TLB for a clear.
    */
   if (!state->tile_aligned_render_area)
      return false;

   /* If this job is running in a subpass other than the first subpass in
    * which this attachment (or view) is used then attachment load operations
    * don't apply.
    */
   if (state->job->first_subpass != first_subpass_idx)
      return false;

   /* The attachment load operation must be CLEAR */
   return load_op == VK_ATTACHMENT_LOAD_OP_CLEAR;
}

static void
cmd_buffer_render_pass_emit_stores(struct v3dv_cmd_buffer *cmd_buffer,
                                   struct v3dv_cl *cl,
                                   uint32_t layer)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   struct v3dv_render_pass *pass = state->pass;
   const struct v3dv_subpass *subpass =
      &pass->subpasses[state->subpass_idx];

   bool has_stores = false;
   bool use_global_zs_clear = false;
   bool use_global_rt_clear = false;

   assert(!pass->multiview_enabled || layer < MAX_MULTIVIEW_VIEW_COUNT);

   /* FIXME: separate stencil */
   uint32_t ds_attachment_idx = subpass->ds_attachment.attachment;
   if (ds_attachment_idx != VK_ATTACHMENT_UNUSED) {
      const struct v3dv_render_pass_attachment *ds_attachment =
         &state->pass->attachments[ds_attachment_idx];

      assert(state->job->first_subpass >= ds_attachment->first_subpass);
      assert(state->subpass_idx >= ds_attachment->first_subpass);
      assert(state->subpass_idx <= ds_attachment->last_subpass);

      /* From the Vulkan spec, VkImageSubresourceRange:
       *
       *   "When an image view of a depth/stencil image is used as a
       *   depth/stencil framebuffer attachment, the aspectMask is ignored
       *   and both depth and stencil image subresources are used."
       *
       * So we ignore the aspects from the subresource range of the image
       * view for the depth/stencil attachment, but we still need to restrict
       * the to aspects compatible with the render pass and the image.
       */
      const VkImageAspectFlags aspects =
         vk_format_aspects(ds_attachment->desc.format);

#if V3D_VERSION <= 42
      /* GFXH-1689: The per-buffer store command's clear buffer bit is broken
       * for depth/stencil.
       *
       * There used to be some confusion regarding the Clear Tile Buffers
       * Z/S bit also being broken, but we confirmed with Broadcom that this
       * is not the case, it was just that some other hardware bugs (that we
       * need to work around, such as GFXH-1461) could cause this bit to behave
       * incorrectly.
       *
       * There used to be another issue where the RTs bit in the Clear Tile
       * Buffers packet also cleared Z/S, but Broadcom confirmed this is
       * fixed since V3D 4.1.
       *
       * So if we have to emit a clear of depth or stencil we don't use
       * the per-buffer store clear bit, even if we need to store the buffers,
       * instead we always have to use the Clear Tile Buffers Z/S bit.
       * If we have configured the job to do early Z/S clearing, then we
       * don't want to emit any Clear Tile Buffers command at all here.
       *
       * Note that GFXH-1689 is not reproduced in the simulator, where
       * using the clear buffer bit in depth/stencil stores works fine.
       */

      /* Only clear once on the first subpass that uses the attachment */
      uint32_t ds_first_subpass = !state->pass->multiview_enabled ?
         ds_attachment->first_subpass :
         ds_attachment->views[layer].first_subpass;

      bool needs_depth_clear =
         check_needs_clear(state,
                           aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                           ds_first_subpass,
                           ds_attachment->desc.loadOp,
                           subpass->do_depth_clear_with_draw);

      bool needs_stencil_clear =
         check_needs_clear(state,
                           aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                           ds_first_subpass,
                           ds_attachment->desc.stencilLoadOp,
                           subpass->do_stencil_clear_with_draw);

      use_global_zs_clear = !state->job->early_zs_clear &&
         (needs_depth_clear || needs_stencil_clear);
#endif
#if V3D_VERSION >= 71
      /* The store command's clear buffer bit cannot be used for Z/S stencil:
       * since V3D 4.5.6 Z/S buffers are automatically cleared between tiles,
       * so we don't want to emit redundant clears here.
       */
      use_global_zs_clear = false;
#endif

      /* Skip the last store if it is not required */
      uint32_t ds_last_subpass = !pass->multiview_enabled ?
         ds_attachment->last_subpass :
         ds_attachment->views[layer].last_subpass;

      bool needs_depth_store =
         v3dv_cmd_buffer_check_needs_store(state,
                                           aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                           ds_last_subpass,
                                           ds_attachment->desc.storeOp);

      bool needs_stencil_store =
         v3dv_cmd_buffer_check_needs_store(state,
                                           aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                                           ds_last_subpass,
                                           ds_attachment->desc.stencilStoreOp);

      /* If we have a resolve, handle it before storing the tile */
      const struct v3dv_cmd_buffer_attachment_state *ds_att_state =
         &state->attachments[ds_attachment_idx];
      if (ds_att_state->use_tlb_resolve) {
         assert(ds_att_state->has_resolve);
         assert(subpass->resolve_depth || subpass->resolve_stencil);
         const uint32_t resolve_attachment_idx =
            subpass->ds_resolve_attachment.attachment;
         assert(resolve_attachment_idx != VK_ATTACHMENT_UNUSED);

         const uint32_t zs_buffer =
            v3dv_zs_buffer(subpass->resolve_depth, subpass->resolve_stencil);
         cmd_buffer_render_pass_emit_store(cmd_buffer, cl,
                                           resolve_attachment_idx, layer,
                                           zs_buffer,
                                           false, false);
         has_stores = true;
      } else if (ds_att_state->has_resolve) {
         /* If we can't use the TLB to implement the resolve we will need to
          * store the attachment so we can implement it later using a blit.
          */
         needs_depth_store = subpass->resolve_depth;
         needs_stencil_store = subpass->resolve_stencil;
      }

      if (needs_depth_store || needs_stencil_store) {
         const uint32_t zs_buffer =
            v3dv_zs_buffer(needs_depth_store, needs_stencil_store);
         cmd_buffer_render_pass_emit_store(cmd_buffer, cl,
                                           ds_attachment_idx, layer,
                                           zs_buffer, false, false);
         has_stores = true;
      }
   }

   for (uint32_t i = 0; i < subpass->color_count; i++) {
      uint32_t attachment_idx = subpass->color_attachments[i].attachment;

      if (attachment_idx == VK_ATTACHMENT_UNUSED)
         continue;

      const struct v3dv_render_pass_attachment *attachment =
         &state->pass->attachments[attachment_idx];

      assert(state->job->first_subpass >= attachment->first_subpass);
      assert(state->subpass_idx >= attachment->first_subpass);
      assert(state->subpass_idx <= attachment->last_subpass);

      /* Only clear once on the first subpass that uses the attachment */
      uint32_t first_subpass = !pass->multiview_enabled ?
         attachment->first_subpass :
         attachment->views[layer].first_subpass;

      bool needs_clear =
         check_needs_clear(state,
                           VK_IMAGE_ASPECT_COLOR_BIT,
                           first_subpass,
                           attachment->desc.loadOp,
                           false);

      /* Skip the last store if it is not required  */
      uint32_t last_subpass = !pass->multiview_enabled ?
         attachment->last_subpass :
         attachment->views[layer].last_subpass;

      bool needs_store =
         v3dv_cmd_buffer_check_needs_store(state,
                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                           last_subpass,
                                           attachment->desc.storeOp);

      /* If we need to resolve this attachment emit that store first. Notice
       * that we must not request a tile buffer clear here in that case, since
       * that would clear the tile buffer before we get to emit the actual
       * color attachment store below, since the clear happens after the
       * store is completed.
       *
       * If the attachment doesn't support TLB resolves (or the render area
       * is not aligned to tile boundaries) then we will have to fallback to
       * doing the resolve in a shader separately after this job, so we will
       * need to store the multisampled attachment even if that wasn't
       * requested by the client.
       */
      const struct v3dv_cmd_buffer_attachment_state *att_state =
         &state->attachments[attachment_idx];
      if (att_state->use_tlb_resolve) {
         assert(att_state->has_resolve);
         const uint32_t resolve_attachment_idx =
            subpass->resolve_attachments[i].attachment;
         cmd_buffer_render_pass_emit_store(cmd_buffer, cl,
                                           resolve_attachment_idx, layer,
                                           RENDER_TARGET_0 + i,
                                           false, true);
         has_stores = true;
      } else if (att_state->has_resolve) {
         needs_store = true;
      }

      /* Emit the color attachment store if needed */
      if (needs_store) {
         cmd_buffer_render_pass_emit_store(cmd_buffer, cl,
                                           attachment_idx, layer,
                                           RENDER_TARGET_0 + i,
                                           needs_clear && !use_global_rt_clear,
                                           false);
         has_stores = true;
      } else if (needs_clear) {
         use_global_rt_clear = true;
      }
   }

   /* We always need to emit at least one dummy store */
   if (!has_stores) {
      cl_emit(cl, STORE_TILE_BUFFER_GENERAL, store) {
         store.buffer_to_store = NONE;
      }
   }

   /* If we have any depth/stencil clears we can't use the per-buffer clear
    * bit and instead we have to emit a single clear of all tile buffers.
    */
   if (use_global_zs_clear || use_global_rt_clear) {
#if V3D_VERSION == 42
      cl_emit(cl, CLEAR_TILE_BUFFERS, clear) {
         clear.clear_z_stencil_buffer = use_global_zs_clear;
         clear.clear_all_render_targets = use_global_rt_clear;
      }
#endif
#if V3D_VERSION >= 71
      cl_emit(cl, CLEAR_RENDER_TARGETS, clear);
#endif
   }
}

static void
cmd_buffer_render_pass_emit_per_tile_rcl(struct v3dv_cmd_buffer *cmd_buffer,
                                         uint32_t layer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   /* Emit the generic list in our indirect state -- the rcl will just
    * have pointers into it.
    */
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(cmd_buffer, NULL);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   cmd_buffer_render_pass_emit_loads(cmd_buffer, cl, layer);

   /* The binner starts out writing tiles assuming that the initial mode
    * is triangles, so make sure that's the case.
    */
   cl_emit(cl, PRIM_LIST_FORMAT, fmt) {
      fmt.primitive_type = LIST_TRIANGLES;
   }

   /* PTB assumes that value to be 0, but hw will not set it. */
   cl_emit(cl, SET_INSTANCEID, set) {
      set.instance_id = 0;
   }

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   cmd_buffer_render_pass_emit_stores(cmd_buffer, cl, layer);

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

static void
cmd_buffer_emit_render_pass_layer_rcl(struct v3dv_cmd_buffer *cmd_buffer,
                                      uint32_t layer)
{
   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   struct v3dv_job *job = cmd_buffer->state.job;
   struct v3dv_cl *rcl = &job->rcl;

   /* If doing multicore binning, we would need to initialize each
    * core's tile list here.
    */
   const struct v3dv_frame_tiling *tiling = &job->frame_tiling;
   const uint32_t tile_alloc_offset =
      64 * layer * tiling->draw_tiles_x * tiling->draw_tiles_y;
   cl_emit(rcl, MULTICORE_RENDERING_TILE_LIST_SET_BASE, list) {
      list.address = v3dv_cl_address(job->tile_alloc, tile_alloc_offset);
   }

   cmd_buffer_render_pass_emit_per_tile_rcl(cmd_buffer, layer);

   uint32_t supertile_w_in_pixels =
      tiling->tile_width * tiling->supertile_width;
   uint32_t supertile_h_in_pixels =
      tiling->tile_height * tiling->supertile_height;
   const uint32_t min_x_supertile =
      state->render_area.offset.x / supertile_w_in_pixels;
   const uint32_t min_y_supertile =
      state->render_area.offset.y / supertile_h_in_pixels;

   uint32_t max_render_x = state->render_area.offset.x;
   if (state->render_area.extent.width > 0)
      max_render_x += state->render_area.extent.width - 1;
   uint32_t max_render_y = state->render_area.offset.y;
   if (state->render_area.extent.height > 0)
      max_render_y += state->render_area.extent.height - 1;
   const uint32_t max_x_supertile = max_render_x / supertile_w_in_pixels;
   const uint32_t max_y_supertile = max_render_y / supertile_h_in_pixels;

   for (int y = min_y_supertile; y <= max_y_supertile; y++) {
      for (int x = min_x_supertile; x <= max_x_supertile; x++) {
         cl_emit(rcl, SUPERTILE_COORDINATES, coords) {
            coords.column_number_in_supertiles = x;
            coords.row_number_in_supertiles = y;
         }
      }
   }
}

static void
set_rcl_early_z_config(struct v3dv_job *job,
                       bool *early_z_disable,
                       uint32_t *early_z_test_and_update_direction)
{
   /* Disable if none of the draw calls in this job enabled EZ */
   if (!job->has_ez_draws) {
      *early_z_disable = true;
      return;
   }

   switch (job->first_ez_state) {
   case V3D_EZ_UNDECIDED:
   case V3D_EZ_LT_LE:
      *early_z_disable = false;
      *early_z_test_and_update_direction = EARLY_Z_DIRECTION_LT_LE;
      break;
   case V3D_EZ_GT_GE:
      *early_z_disable = false;
      *early_z_test_and_update_direction = EARLY_Z_DIRECTION_GT_GE;
      break;
   case V3D_EZ_DISABLED:
      *early_z_disable = true;
      break;
   }
}

/* Note that for v71, render target cfg packets has just one field that
 * combined the internal type and clamp mode. For simplicity we keep just one
 * helper.
 *
 * Note: rt_type is in fact a "enum V3DX(Internal_Type)".
 *
 * FIXME: for v71 we are not returning all the possible combinations for
 * render target internal type and clamp. For example for int types we are
 * always using clamp int, and for 16f we are using clamp none or pos (that
 * seems to be the equivalent for no-clamp on 4.2), but not pq or hlg. In
 * summary right now we are just porting what we were doing on 4.2
 */
uint32_t
v3dX(clamp_for_format_and_type)(uint32_t rt_type,
                                VkFormat vk_format)
{
#if V3D_VERSION == 42
   if (vk_format_is_int(vk_format))
      return V3D_RENDER_TARGET_CLAMP_INT;
   else if (vk_format_is_srgb(vk_format))
      return V3D_RENDER_TARGET_CLAMP_NORM;
   else
      return V3D_RENDER_TARGET_CLAMP_NONE;
#endif
#if V3D_VERSION >= 71
   switch (rt_type) {
   case V3D_INTERNAL_TYPE_8I:
      return V3D_RENDER_TARGET_TYPE_CLAMP_8I_CLAMPED;
   case V3D_INTERNAL_TYPE_8UI:
      return V3D_RENDER_TARGET_TYPE_CLAMP_8UI_CLAMPED;
   case V3D_INTERNAL_TYPE_8:
      return V3D_RENDER_TARGET_TYPE_CLAMP_8;
   case V3D_INTERNAL_TYPE_16I:
      return V3D_RENDER_TARGET_TYPE_CLAMP_16I_CLAMPED;
   case V3D_INTERNAL_TYPE_16UI:
      return V3D_RENDER_TARGET_TYPE_CLAMP_16UI_CLAMPED;
   case V3D_INTERNAL_TYPE_16F:
      return vk_format_is_srgb(vk_format) ?
         V3D_RENDER_TARGET_TYPE_CLAMP_16F_CLAMP_NORM :
         V3D_RENDER_TARGET_TYPE_CLAMP_16F;
   case V3D_INTERNAL_TYPE_32I:
      return V3D_RENDER_TARGET_TYPE_CLAMP_32I_CLAMPED;
   case V3D_INTERNAL_TYPE_32UI:
      return V3D_RENDER_TARGET_TYPE_CLAMP_32UI_CLAMPED;
   case V3D_INTERNAL_TYPE_32F:
      return V3D_RENDER_TARGET_TYPE_CLAMP_32F;
   default:
      unreachable("Unknown internal render target type");
   }

   return V3D_RENDER_TARGET_TYPE_CLAMP_INVALID;
#endif
}

static void
cmd_buffer_render_pass_setup_render_target(struct v3dv_cmd_buffer *cmd_buffer,
                                           int rt,
                                           uint32_t *rt_bpp,
#if V3D_VERSION == 42
                                           uint32_t *rt_type,
                                           uint32_t *rt_clamp)
#else
                                           uint32_t *rt_type_clamp)
#endif
{
   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   assert(state->subpass_idx < state->pass->subpass_count);
   const struct v3dv_subpass *subpass =
      &state->pass->subpasses[state->subpass_idx];

   if (rt >= subpass->color_count)
      return;

   struct v3dv_subpass_attachment *attachment = &subpass->color_attachments[rt];
   const uint32_t attachment_idx = attachment->attachment;
   if (attachment_idx == VK_ATTACHMENT_UNUSED)
      return;

   assert(attachment_idx < state->framebuffer->attachment_count &&
          attachment_idx < state->attachment_alloc_count);
   struct v3dv_image_view *iview = state->attachments[attachment_idx].image_view;
   assert(vk_format_is_color(iview->vk.format));

   assert(iview->plane_count == 1);
   *rt_bpp = iview->planes[0].internal_bpp;
#if V3D_VERSION == 42
   *rt_type = iview->planes[0].internal_type;
   *rt_clamp = v3dX(clamp_for_format_and_type)(iview->planes[0].internal_type,
                                               iview->vk.format);
#endif
#if V3D_VERSION >= 71
   *rt_type_clamp = v3dX(clamp_for_format_and_type)(iview->planes[0].internal_type,
                                                    iview->vk.format);
#endif
}

void
v3dX(cmd_buffer_emit_render_pass_rcl)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   const struct v3dv_framebuffer *framebuffer = state->framebuffer;

   /* We can't emit the RCL until we have a framebuffer, which we may not have
    * if we are recording a secondary command buffer. In that case, we will
    * have to wait until vkCmdExecuteCommands is called from a primary command
    * buffer.
    */
   if (!framebuffer) {
      assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
      return;
   }

   const struct v3dv_frame_tiling *tiling = &job->frame_tiling;

   const uint32_t fb_layers = job->frame_tiling.layers;

   v3dv_cl_ensure_space_with_branch(&job->rcl, 200 +
                                    MAX2(fb_layers, 1) * 256 *
                                    cl_packet_length(SUPERTILE_COORDINATES));
   v3dv_return_if_oom(cmd_buffer, NULL);

   assert(state->subpass_idx < state->pass->subpass_count);
   const struct v3dv_render_pass *pass = state->pass;
   const struct v3dv_subpass *subpass = &pass->subpasses[state->subpass_idx];
   struct v3dv_cl *rcl = &job->rcl;

   /* Common config must be the first TILE_RENDERING_MODE_CFG and
    * Z_STENCIL_CLEAR_VALUES must be last. The ones in between are optional
    * updates to the previous HW state.
    */
   bool do_early_zs_clear = false;
   const uint32_t ds_attachment_idx = subpass->ds_attachment.attachment;
   assert(!tiling->msaa || !tiling->double_buffer);
   cl_emit(rcl, TILE_RENDERING_MODE_CFG_COMMON, config) {
      config.image_width_pixels = framebuffer->width;
      config.image_height_pixels = framebuffer->height;
      config.number_of_render_targets = MAX2(subpass->color_count, 1);
      config.multisample_mode_4x = tiling->msaa;
      config.double_buffer_in_non_ms_mode = tiling->double_buffer;
#if V3D_VERSION == 42
      config.maximum_bpp_of_all_render_targets = tiling->internal_bpp;
#endif
#if V3D_VERSION >= 71
      config.log2_tile_width = log2_tile_size(tiling->tile_width);
      config.log2_tile_height = log2_tile_size(tiling->tile_height);
      /* FIXME: ideallly we would like next assert on the packet header (as is
       * general, so also applies to GL). We would need to expand
       * gen_pack_header for that.
       */
      assert(config.log2_tile_width == config.log2_tile_height ||
             config.log2_tile_width == config.log2_tile_height + 1);
#endif

      if (ds_attachment_idx != VK_ATTACHMENT_UNUSED) {
         const struct v3dv_image_view *iview =
            state->attachments[ds_attachment_idx].image_view;

         /* At this point the image view should be single-plane. But note that
          * the underlying image can be multi-plane, and the image view refer
          * to one specific plane.
          */
         assert(iview->plane_count == 1);
         assert(iview->format->plane_count == 1);
         config.internal_depth_type = iview->planes[0].internal_type;

         set_rcl_early_z_config(job,
                                &config.early_z_disable,
                                &config.early_z_test_and_update_direction);

         /* Early-Z/S clear can be enabled if the job is clearing and not
          * storing (or loading) depth. If a stencil aspect is also present
          * we have the same requirements for it, however, in this case we
          * can accept stencil loadOp DONT_CARE as well, so instead of
          * checking that stencil is cleared we check that is not loaded.
          *
          * Early-Z/S clearing is independent of Early Z/S testing, so it is
          * possible to enable one but not the other so long as their
          * respective requirements are met.
          *
          * From V3D 4.5.6, Z/S buffers are always cleared automatically
          * between tiles, but we still want to enable early ZS clears
          * when Z/S are not loaded or stored.
          */
         struct v3dv_render_pass_attachment *ds_attachment =
            &pass->attachments[ds_attachment_idx];

         const VkImageAspectFlags ds_aspects =
            vk_format_aspects(ds_attachment->desc.format);

         bool needs_depth_store =
            v3dv_cmd_buffer_check_needs_store(state,
                                              ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                              ds_attachment->last_subpass,
                                              ds_attachment->desc.storeOp) ||
                                              subpass->resolve_depth;
#if V3D_VERSION <= 42
         bool needs_depth_clear =
            check_needs_clear(state,
                              ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                              ds_attachment->first_subpass,
                              ds_attachment->desc.loadOp,
                              subpass->do_depth_clear_with_draw);

         do_early_zs_clear = needs_depth_clear && !needs_depth_store;
#endif
#if V3D_VERSION >= 71
         bool needs_depth_load =
            v3dv_cmd_buffer_check_needs_load(state,
                                             ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                             ds_attachment->first_subpass,
                                             ds_attachment->desc.loadOp,
                                             ds_attachment->last_subpass,
                                             ds_attachment->desc.storeOp);
         do_early_zs_clear = !needs_depth_load && !needs_depth_store;
#endif

         if (do_early_zs_clear &&
             vk_format_has_stencil(ds_attachment->desc.format)) {
            bool needs_stencil_load =
               v3dv_cmd_buffer_check_needs_load(state,
                                                ds_aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                                                ds_attachment->first_subpass,
                                                ds_attachment->desc.stencilLoadOp,
                                                ds_attachment->last_subpass,
                                                ds_attachment->desc.stencilStoreOp);

            bool needs_stencil_store =
               v3dv_cmd_buffer_check_needs_store(state,
                                                 ds_aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                                                 ds_attachment->last_subpass,
                                                 ds_attachment->desc.stencilStoreOp) ||
               subpass->resolve_stencil;

            do_early_zs_clear = !needs_stencil_load && !needs_stencil_store;
         }

         config.early_depth_stencil_clear = do_early_zs_clear;
      } else {
         config.early_z_disable = true;
      }
   }

   /* If we enabled early Z/S clear, then we can't emit any "Clear Tile Buffers"
    * commands with the Z/S bit set, so keep track of whether we enabled this
    * in the job so we can skip these later.
    */
   job->early_zs_clear = do_early_zs_clear;

#if V3D_VERSION >= 71
   uint32_t base_addr = 0;
#endif
   for (uint32_t i = 0; i < subpass->color_count; i++) {
      uint32_t attachment_idx = subpass->color_attachments[i].attachment;
      if (attachment_idx == VK_ATTACHMENT_UNUSED) {
#if V3D_VERSION >= 71
         cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
            rt.render_target_number = i;
            rt.stride = 1; /* Unused */
         }
#endif
         continue;
      }

      struct v3dv_image_view *iview =
         state->attachments[attachment_idx].image_view;
      assert(iview->plane_count == 1);

      const struct v3dv_image *image = (struct v3dv_image *) iview->vk.image;

      uint8_t plane = v3dv_plane_from_aspect(iview->vk.aspects);
      const struct v3d_resource_slice *slice =
         &image->planes[plane].slices[iview->vk.base_mip_level];

      UNUSED const uint32_t *clear_color =
         &state->attachments[attachment_idx].clear_value.color[0];

      UNUSED uint32_t clear_pad = 0;
      if (slice->tiling == V3D_TILING_UIF_NO_XOR ||
          slice->tiling == V3D_TILING_UIF_XOR) {
         int uif_block_height = v3d_utile_height(image->planes[plane].cpp) * 2;

         uint32_t implicit_padded_height =
            align(framebuffer->height, uif_block_height) / uif_block_height;

         if (slice->padded_height_of_output_image_in_uif_blocks -
             implicit_padded_height >= 15) {
            clear_pad = slice->padded_height_of_output_image_in_uif_blocks;
         }
      }

#if V3D_VERSION == 42
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART1, clear) {
         clear.clear_color_low_32_bits = clear_color[0];
         clear.clear_color_next_24_bits = clear_color[1] & 0xffffff;
         clear.render_target_number = i;
      };

      if (iview->planes[0].internal_bpp >= V3D_INTERNAL_BPP_64) {
         cl_emit(rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART2, clear) {
            clear.clear_color_mid_low_32_bits =
               ((clear_color[1] >> 24) | (clear_color[2] << 8));
            clear.clear_color_mid_high_24_bits =
               ((clear_color[2] >> 24) | ((clear_color[3] & 0xffff) << 8));
            clear.render_target_number = i;
         };
      }

      if (iview->planes[0].internal_bpp >= V3D_INTERNAL_BPP_128 || clear_pad) {
         cl_emit(rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART3, clear) {
            clear.uif_padded_height_in_uif_blocks = clear_pad;
            clear.clear_color_high_16_bits = clear_color[3] >> 16;
            clear.render_target_number = i;
         };
      }
#endif

#if V3D_VERSION >= 71
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
         rt.clear_color_low_bits = clear_color[0];
         cmd_buffer_render_pass_setup_render_target(cmd_buffer, i, &rt.internal_bpp,
                                                    &rt.internal_type_and_clamping);
         rt.stride =
            v3d_compute_rt_row_row_stride_128_bits(tiling->tile_width,
                                                   v3d_internal_bpp_words(rt.internal_bpp));
         rt.base_address = base_addr;
         rt.render_target_number = i;

         /* base_addr in multiples of 512 bits. We divide by 8 because stride
          * is in 128-bit units, but it is packing 2 rows worth of data, so we
          * need to divide it by 2 so it is only 1 row, and then again by 4 so
          * it is in 512-bit units.
          */
         base_addr += (tiling->tile_height * rt.stride) / 8;
      }

      if (iview->planes[0].internal_bpp >= V3D_INTERNAL_BPP_64) {
         cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART2, rt) {
            rt.clear_color_mid_bits = /* 40 bits (32 + 8)  */
               ((uint64_t) clear_color[1]) |
               (((uint64_t) (clear_color[2] & 0xff)) << 32);
            rt.render_target_number = i;
         }
      }

      if (iview->planes[0].internal_bpp >= V3D_INTERNAL_BPP_128) {
         cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART3, rt) {
            rt.clear_color_top_bits = /* 56 bits (24 + 32) */
               (((uint64_t) (clear_color[2] & 0xffffff00)) >> 8) |
               (((uint64_t) (clear_color[3])) << 24);
            rt.render_target_number = i;
         }
      }
#endif
   }

#if V3D_VERSION >= 71
   /* If we don't have any color RTs, we still need to emit one and flag
    * it as not used using stride = 1.
    */
   if (subpass->color_count == 0) {
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
         rt.stride = 1;
      }
   }
#endif

#if V3D_VERSION == 42
   cl_emit(rcl, TILE_RENDERING_MODE_CFG_COLOR, rt) {
      cmd_buffer_render_pass_setup_render_target
         (cmd_buffer, 0, &rt.render_target_0_internal_bpp,
          &rt.render_target_0_internal_type, &rt.render_target_0_clamp);
      cmd_buffer_render_pass_setup_render_target
         (cmd_buffer, 1, &rt.render_target_1_internal_bpp,
          &rt.render_target_1_internal_type, &rt.render_target_1_clamp);
      cmd_buffer_render_pass_setup_render_target
         (cmd_buffer, 2, &rt.render_target_2_internal_bpp,
          &rt.render_target_2_internal_type, &rt.render_target_2_clamp);
      cmd_buffer_render_pass_setup_render_target
         (cmd_buffer, 3, &rt.render_target_3_internal_bpp,
          &rt.render_target_3_internal_type, &rt.render_target_3_clamp);
   }
#endif

   /* Ends rendering mode config. */
   if (ds_attachment_idx != VK_ATTACHMENT_UNUSED) {
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_ZS_CLEAR_VALUES, clear) {
         clear.z_clear_value =
            state->attachments[ds_attachment_idx].clear_value.z;
         clear.stencil_clear_value =
            state->attachments[ds_attachment_idx].clear_value.s;
      };
   } else {
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_ZS_CLEAR_VALUES, clear) {
         clear.z_clear_value = 1.0f;
         clear.stencil_clear_value = 0;
      };
   }

   /* Always set initial block size before the first branch, which needs
    * to match the value from binning mode config.
    */
   cl_emit(rcl, TILE_LIST_INITIAL_BLOCK_SIZE, init) {
      init.use_auto_chained_tile_lists = true;
      init.size_of_first_block_in_chained_tile_lists =
         TILE_ALLOCATION_BLOCK_SIZE_64B;
   }

   cl_emit(rcl, MULTICORE_RENDERING_SUPERTILE_CFG, config) {
      config.number_of_bin_tile_lists = 1;
      config.total_frame_width_in_tiles = tiling->draw_tiles_x;
      config.total_frame_height_in_tiles = tiling->draw_tiles_y;

      config.supertile_width_in_tiles = tiling->supertile_width;
      config.supertile_height_in_tiles = tiling->supertile_height;

      config.total_frame_width_in_supertiles =
         tiling->frame_width_in_supertiles;
      config.total_frame_height_in_supertiles =
         tiling->frame_height_in_supertiles;
   }

   /* Emit an initial clear of the tile buffers. This is necessary
    * for any buffers that should be cleared (since clearing
    * normally happens at the *end* of the generic tile list), but
    * it's also nice to clear everything so the first tile doesn't
    * inherit any contents from some previous frame.
    *
    * Also, implement the GFXH-1742 workaround. There's a race in
    * the HW between the RCL updating the TLB's internal type/size
    * and the spawning of the QPU instances using the TLB's current
    * internal type/size. To make sure the QPUs get the right
    * state, we need 1 dummy store in between internal type/size
    * changes on V3D 3.x, and 2 dummy stores on 4.x.
    */
   for (int i = 0; i < 2; i++) {
      cl_emit(rcl, TILE_COORDINATES, coords);
      cl_emit(rcl, END_OF_LOADS, end);
      cl_emit(rcl, STORE_TILE_BUFFER_GENERAL, store) {
         store.buffer_to_store = NONE;
      }
      if (cmd_buffer->state.tile_aligned_render_area &&
          (i == 0 || v3dv_do_double_initial_tile_clear(tiling))) {
#if V3D_VERSION == 42
         cl_emit(rcl, CLEAR_TILE_BUFFERS, clear) {
            clear.clear_z_stencil_buffer = !job->early_zs_clear;
            clear.clear_all_render_targets = true;
         }
#endif
#if V3D_VERSION >= 71
         cl_emit(rcl, CLEAR_RENDER_TARGETS, clear_rt);
#endif
      }
      cl_emit(rcl, END_OF_TILE_MARKER, end);
   }

   cl_emit(rcl, FLUSH_VCD_CACHE, flush);

   for (int layer = 0; layer < MAX2(1, fb_layers); layer++) {
      if (subpass->view_mask == 0 || (subpass->view_mask & (1u << layer)))
         cmd_buffer_emit_render_pass_layer_rcl(cmd_buffer, layer);
   }

   cl_emit(rcl, END_OF_RENDERING, end);
}

void
v3dX(viewport_compute_xform)(const VkViewport *viewport,
                            float scale[3],
                            float translate[3])
{
   float x = viewport->x;
   float y = viewport->y;
   float half_width = 0.5f * viewport->width;
   float half_height = 0.5f * viewport->height;
   double n = viewport->minDepth;
   double f = viewport->maxDepth;

   scale[0] = half_width;
   translate[0] = half_width + x;
   scale[1] = half_height;
   translate[1] = half_height + y;

   scale[2] = (f - n);
   translate[2] = n;

   /* It seems that if the scale is small enough the hardware won't clip
    * correctly so we work around this my choosing the smallest scale that
    * seems to work.
    *
    * This case is exercised by CTS:
    * dEQP-VK.draw.renderpass.inverted_depth_ranges.nodepthclamp_deltazero
    *
    * V3D 7.x fixes this by using the new
    * CLIPPER_Z_SCALE_AND_OFFSET_NO_GUARDBAND.
    */
#if V3D_VERSION <= 42
   const float min_abs_scale = 0.0005f;
   if (fabs(scale[2]) < min_abs_scale)
      scale[2] = scale[2] < 0 ? -min_abs_scale : min_abs_scale;
#endif
}

void
v3dX(cmd_buffer_emit_viewport)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_dynamic_state *dynamic = &cmd_buffer->state.dynamic;
   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   assert(pipeline);

   /* FIXME: right now we only support one viewport. viewporst[0] would work
    * now, would need to change if we allow multiple viewports
    */
   float *vptranslate = dynamic->viewport.translate[0];
   float *vpscale = dynamic->viewport.scale[0];

   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   const uint32_t required_cl_size =
      cl_packet_length(CLIPPER_XY_SCALING) +
      cl_packet_length(CLIPPER_Z_SCALE_AND_OFFSET) +
      cl_packet_length(CLIPPER_Z_MIN_MAX_CLIPPING_PLANES) +
      cl_packet_length(VIEWPORT_OFFSET);
   v3dv_cl_ensure_space_with_branch(&job->bcl, required_cl_size);
   v3dv_return_if_oom(cmd_buffer, NULL);

#if V3D_VERSION == 42
   cl_emit(&job->bcl, CLIPPER_XY_SCALING, clip) {
      clip.viewport_half_width_in_1_256th_of_pixel = vpscale[0] * 256.0f;
      clip.viewport_half_height_in_1_256th_of_pixel = vpscale[1] * 256.0f;
   }
#endif
#if V3D_VERSION >= 71
   cl_emit(&job->bcl, CLIPPER_XY_SCALING, clip) {
      clip.viewport_half_width_in_1_64th_of_pixel = vpscale[0] * 64.0f;
      clip.viewport_half_height_in_1_64th_of_pixel = vpscale[1] * 64.0f;
   }
#endif

   float translate_z, scale_z;
   v3dv_cmd_buffer_state_get_viewport_z_xform(&cmd_buffer->state, 0,
                                              &translate_z, &scale_z);

#if V3D_VERSION == 42
   cl_emit(&job->bcl, CLIPPER_Z_SCALE_AND_OFFSET, clip) {
      clip.viewport_z_offset_zc_to_zs = translate_z;
      clip.viewport_z_scale_zc_to_zs = scale_z;
   }
#endif

#if V3D_VERSION >= 71
   /* If the Z scale is too small guardband clipping may not clip correctly */
   if (fabsf(scale_z) < 0.01f) {
      cl_emit(&job->bcl, CLIPPER_Z_SCALE_AND_OFFSET_NO_GUARDBAND, clip) {
         clip.viewport_z_offset_zc_to_zs = translate_z;
         clip.viewport_z_scale_zc_to_zs = scale_z;
      }
   } else {
      cl_emit(&job->bcl, CLIPPER_Z_SCALE_AND_OFFSET, clip) {
         clip.viewport_z_offset_zc_to_zs = translate_z;
         clip.viewport_z_scale_zc_to_zs = scale_z;
      }
   }
#endif

   cl_emit(&job->bcl, CLIPPER_Z_MIN_MAX_CLIPPING_PLANES, clip) {
      /* Vulkan's default Z NDC is [0..1]. If 'negative_one_to_one' is enabled,
       * we are using OpenGL's [-1, 1] instead.
       */
      float z1 = pipeline->negative_one_to_one ? translate_z - scale_z :
                                                 translate_z;
      float z2 = translate_z + scale_z;
      clip.minimum_zw = MIN2(z1, z2);
      clip.maximum_zw = MAX2(z1, z2);
   }

   cl_emit(&job->bcl, VIEWPORT_OFFSET, vp) {
      float vp_fine_x = vptranslate[0];
      float vp_fine_y = vptranslate[1];
      int32_t vp_coarse_x = 0;
      int32_t vp_coarse_y = 0;

      /* The fine coordinates must be unsigned, but coarse can be signed */
      if (unlikely(vp_fine_x < 0)) {
         int32_t blocks_64 = DIV_ROUND_UP(fabsf(vp_fine_x), 64);
         vp_fine_x += 64.0f * blocks_64;
         vp_coarse_x -= blocks_64;
      }

      if (unlikely(vp_fine_y < 0)) {
         int32_t blocks_64 = DIV_ROUND_UP(fabsf(vp_fine_y), 64);
         vp_fine_y += 64.0f * blocks_64;
         vp_coarse_y -= blocks_64;
      }

      vp.fine_x = vp_fine_x;
      vp.fine_y = vp_fine_y;
      vp.coarse_x = vp_coarse_x;
      vp.coarse_y = vp_coarse_y;
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_VIEWPORT;
}

void
v3dX(cmd_buffer_emit_stencil)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   struct v3dv_dynamic_state *dynamic_state = &cmd_buffer->state.dynamic;

   const uint32_t dynamic_stencil_states = V3DV_DYNAMIC_STENCIL_COMPARE_MASK |
      V3DV_DYNAMIC_STENCIL_WRITE_MASK |
      V3DV_DYNAMIC_STENCIL_REFERENCE;

   v3dv_cl_ensure_space_with_branch(&job->bcl,
                                    2 * cl_packet_length(STENCIL_CFG));
   v3dv_return_if_oom(cmd_buffer, NULL);

   bool emitted_stencil = false;
   for (uint32_t i = 0; i < 2; i++) {
      if (pipeline->emit_stencil_cfg[i]) {
         if (dynamic_state->mask & dynamic_stencil_states) {
            cl_emit_with_prepacked(&job->bcl, STENCIL_CFG,
                                   pipeline->stencil_cfg[i], config) {
               if (dynamic_state->mask & V3DV_DYNAMIC_STENCIL_COMPARE_MASK) {
                  config.stencil_test_mask =
                     i == 0 ? dynamic_state->stencil_compare_mask.front :
                     dynamic_state->stencil_compare_mask.back;
               }
               if (dynamic_state->mask & V3DV_DYNAMIC_STENCIL_WRITE_MASK) {
                  config.stencil_write_mask =
                     i == 0 ? dynamic_state->stencil_write_mask.front :
                     dynamic_state->stencil_write_mask.back;
               }
               if (dynamic_state->mask & V3DV_DYNAMIC_STENCIL_REFERENCE) {
                  config.stencil_ref_value =
                     i == 0 ? dynamic_state->stencil_reference.front :
                     dynamic_state->stencil_reference.back;
               }
            }
         } else {
            cl_emit_prepacked(&job->bcl, &pipeline->stencil_cfg[i]);
         }

         emitted_stencil = true;
      }
   }

   if (emitted_stencil) {
      const uint32_t dynamic_stencil_dirty_flags =
         V3DV_CMD_DIRTY_STENCIL_COMPARE_MASK |
         V3DV_CMD_DIRTY_STENCIL_WRITE_MASK |
         V3DV_CMD_DIRTY_STENCIL_REFERENCE;
      cmd_buffer->state.dirty &= ~dynamic_stencil_dirty_flags;
   }
}

void
v3dX(cmd_buffer_emit_depth_bias)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   assert(pipeline);

   if (!pipeline->depth_bias.enabled)
      return;

   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(DEPTH_OFFSET));
   v3dv_return_if_oom(cmd_buffer, NULL);

   struct v3dv_dynamic_state *dynamic = &cmd_buffer->state.dynamic;
   cl_emit(&job->bcl, DEPTH_OFFSET, bias) {
      bias.depth_offset_factor = dynamic->depth_bias.slope_factor;
      bias.depth_offset_units = dynamic->depth_bias.constant_factor;
#if V3D_VERSION <= 42
      if (pipeline->depth_bias.is_z16)
         bias.depth_offset_units *= 256.0f;
#endif
      bias.limit = dynamic->depth_bias.depth_bias_clamp;
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_DEPTH_BIAS;
}

void
v3dX(cmd_buffer_emit_depth_bounds)(struct v3dv_cmd_buffer *cmd_buffer)
{
   /* No depthBounds support for v42, so this method is empty in that case.
    *
    * Note that this method is being called as v3dv_job_init flags all state
    * as dirty. See FIXME note in v3dv_job_init.
    */

#if V3D_VERSION >= 71
   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   assert(pipeline);

   if (!pipeline->depth_bounds_test_enabled)
      return;

   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(DEPTH_BOUNDS_TEST_LIMITS));
   v3dv_return_if_oom(cmd_buffer, NULL);

   struct v3dv_dynamic_state *dynamic = &cmd_buffer->state.dynamic;
   cl_emit(&job->bcl, DEPTH_BOUNDS_TEST_LIMITS, bounds) {
      bounds.lower_test_limit = dynamic->depth_bounds.min;
      bounds.upper_test_limit = dynamic->depth_bounds.max;
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_DEPTH_BOUNDS;
#endif
}

void
v3dX(cmd_buffer_emit_line_width)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(LINE_WIDTH));
   v3dv_return_if_oom(cmd_buffer, NULL);

   cl_emit(&job->bcl, LINE_WIDTH, line) {
      line.line_width = cmd_buffer->state.dynamic.line_width;
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_LINE_WIDTH;
}

void
v3dX(cmd_buffer_emit_sample_state)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   assert(pipeline);

   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(SAMPLE_STATE));
   v3dv_return_if_oom(cmd_buffer, NULL);

   cl_emit(&job->bcl, SAMPLE_STATE, state) {
      state.coverage = 1.0f;
      state.mask = pipeline->sample_mask;
   }
}

void
v3dX(cmd_buffer_emit_blend)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   assert(pipeline);

   const struct v3d_device_info *devinfo = &cmd_buffer->device->devinfo;
   const uint32_t max_color_rts = V3D_MAX_RENDER_TARGETS(devinfo->ver);

   const uint32_t blend_packets_size =
      cl_packet_length(BLEND_ENABLES) +
      cl_packet_length(BLEND_CONSTANT_COLOR) +
      cl_packet_length(BLEND_CFG) * max_color_rts;

   v3dv_cl_ensure_space_with_branch(&job->bcl, blend_packets_size);
   v3dv_return_if_oom(cmd_buffer, NULL);

   if (cmd_buffer->state.dirty & V3DV_CMD_DIRTY_PIPELINE) {
      if (pipeline->blend.enables) {
         cl_emit(&job->bcl, BLEND_ENABLES, enables) {
            enables.mask = pipeline->blend.enables;
         }
      }

      for (uint32_t i = 0; i < max_color_rts; i++) {
         if (pipeline->blend.enables & (1 << i))
            cl_emit_prepacked(&job->bcl, &pipeline->blend.cfg[i]);
      }
   }

   if (pipeline->blend.needs_color_constants &&
       cmd_buffer->state.dirty & V3DV_CMD_DIRTY_BLEND_CONSTANTS) {
      struct v3dv_dynamic_state *dynamic = &cmd_buffer->state.dynamic;
      cl_emit(&job->bcl, BLEND_CONSTANT_COLOR, color) {
         color.red_f16 = _mesa_float_to_half(dynamic->blend_constants[0]);
         color.green_f16 = _mesa_float_to_half(dynamic->blend_constants[1]);
         color.blue_f16 = _mesa_float_to_half(dynamic->blend_constants[2]);
         color.alpha_f16 = _mesa_float_to_half(dynamic->blend_constants[3]);
      }
      cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_BLEND_CONSTANTS;
   }
}

void
v3dX(cmd_buffer_emit_color_write_mask)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(COLOR_WRITE_MASKS));

   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   struct v3dv_dynamic_state *dynamic = &cmd_buffer->state.dynamic;
   uint32_t color_write_mask = ~dynamic->color_write_enable |
                               pipeline->blend.color_write_masks;
#if V3D_VERSION <= 42
   /* Only 4 RTs */
   color_write_mask &= 0xffff;
#endif

   cl_emit(&job->bcl, COLOR_WRITE_MASKS, mask) {
      mask.mask = color_write_mask;
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_COLOR_WRITE_ENABLE;
}

static void
emit_flat_shade_flags(struct v3dv_job *job,
                      int varying_offset,
                      uint32_t varyings,
                      enum V3DX(Varying_Flags_Action) lower,
                      enum V3DX(Varying_Flags_Action) higher)
{
   v3dv_cl_ensure_space_with_branch(&job->bcl,
                                    cl_packet_length(FLAT_SHADE_FLAGS));
   v3dv_return_if_oom(NULL, job);

   cl_emit(&job->bcl, FLAT_SHADE_FLAGS, flags) {
      flags.varying_offset_v0 = varying_offset;
      flags.flat_shade_flags_for_varyings_v024 = varyings;
      flags.action_for_flat_shade_flags_of_lower_numbered_varyings = lower;
      flags.action_for_flat_shade_flags_of_higher_numbered_varyings = higher;
   }
}

static void
emit_noperspective_flags(struct v3dv_job *job,
                         int varying_offset,
                         uint32_t varyings,
                         enum V3DX(Varying_Flags_Action) lower,
                         enum V3DX(Varying_Flags_Action) higher)
{
   v3dv_cl_ensure_space_with_branch(&job->bcl,
                                    cl_packet_length(NON_PERSPECTIVE_FLAGS));
   v3dv_return_if_oom(NULL, job);

   cl_emit(&job->bcl, NON_PERSPECTIVE_FLAGS, flags) {
      flags.varying_offset_v0 = varying_offset;
      flags.non_perspective_flags_for_varyings_v024 = varyings;
      flags.action_for_non_perspective_flags_of_lower_numbered_varyings = lower;
      flags.action_for_non_perspective_flags_of_higher_numbered_varyings = higher;
   }
}

static void
emit_centroid_flags(struct v3dv_job *job,
                    int varying_offset,
                    uint32_t varyings,
                    enum V3DX(Varying_Flags_Action) lower,
                    enum V3DX(Varying_Flags_Action) higher)
{
   v3dv_cl_ensure_space_with_branch(&job->bcl,
                                    cl_packet_length(CENTROID_FLAGS));
   v3dv_return_if_oom(NULL, job);

   cl_emit(&job->bcl, CENTROID_FLAGS, flags) {
      flags.varying_offset_v0 = varying_offset;
      flags.centroid_flags_for_varyings_v024 = varyings;
      flags.action_for_centroid_flags_of_lower_numbered_varyings = lower;
      flags.action_for_centroid_flags_of_higher_numbered_varyings = higher;
   }
}

static bool
emit_varying_flags(struct v3dv_job *job,
                   uint32_t num_flags,
                   const uint32_t *flags,
                   void (*flag_emit_callback)(struct v3dv_job *job,
                                              int varying_offset,
                                              uint32_t flags,
                                              enum V3DX(Varying_Flags_Action) lower,
                                              enum V3DX(Varying_Flags_Action) higher))
{
   bool emitted_any = false;
   for (int i = 0; i < num_flags; i++) {
      if (!flags[i])
         continue;

      if (emitted_any) {
         flag_emit_callback(job, i, flags[i],
                            V3D_VARYING_FLAGS_ACTION_UNCHANGED,
                            V3D_VARYING_FLAGS_ACTION_UNCHANGED);
      } else if (i == 0) {
         flag_emit_callback(job, i, flags[i],
                            V3D_VARYING_FLAGS_ACTION_UNCHANGED,
                            V3D_VARYING_FLAGS_ACTION_ZEROED);
      } else {
         flag_emit_callback(job, i, flags[i],
                            V3D_VARYING_FLAGS_ACTION_ZEROED,
                            V3D_VARYING_FLAGS_ACTION_ZEROED);
      }

      emitted_any = true;
   }

   return emitted_any;
}

void
v3dX(cmd_buffer_emit_varyings_state)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;

   struct v3d_fs_prog_data *prog_data_fs =
      pipeline->shared_data->variants[BROADCOM_SHADER_FRAGMENT]->prog_data.fs;

   const uint32_t num_flags =
      ARRAY_SIZE(prog_data_fs->flat_shade_flags);
   const uint32_t *flat_shade_flags = prog_data_fs->flat_shade_flags;
   const uint32_t *noperspective_flags =  prog_data_fs->noperspective_flags;
   const uint32_t *centroid_flags = prog_data_fs->centroid_flags;

   if (!emit_varying_flags(job, num_flags, flat_shade_flags,
                           emit_flat_shade_flags)) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(ZERO_ALL_FLAT_SHADE_FLAGS));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, ZERO_ALL_FLAT_SHADE_FLAGS, flags);
   }

   if (!emit_varying_flags(job, num_flags, noperspective_flags,
                           emit_noperspective_flags)) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(ZERO_ALL_NON_PERSPECTIVE_FLAGS));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, ZERO_ALL_NON_PERSPECTIVE_FLAGS, flags);
   }

   if (!emit_varying_flags(job, num_flags, centroid_flags,
                           emit_centroid_flags)) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(ZERO_ALL_CENTROID_FLAGS));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, ZERO_ALL_CENTROID_FLAGS, flags);
   }
}

/* Updates job early Z state tracking. Returns False if EZ must be disabled
 * for the current draw call.
 */
static bool
job_update_ez_state(struct v3dv_job *job,
                    struct v3dv_pipeline *pipeline,
                    struct v3dv_cmd_buffer *cmd_buffer)
{
   /* If first_ez_state is V3D_EZ_DISABLED it means that we have already
    * determined that we should disable EZ completely for all draw calls in
    * this job. This will cause us to disable EZ for the entire job in the
    * Tile Rendering Mode RCL packet and when we do that we need to make sure
    * we never emit a draw call in the job with EZ enabled in the CFG_BITS
    * packet, so ez_state must also be V3D_EZ_DISABLED;
    */
   if (job->first_ez_state == V3D_EZ_DISABLED) {
      assert(job->ez_state == V3D_EZ_DISABLED);
      return false;
   }

   /* If ez_state is V3D_EZ_DISABLED it means that we have already decided
    * that EZ must be disabled for the remaining of the frame.
    */
   if (job->ez_state == V3D_EZ_DISABLED)
      return false;

   /* This is part of the pre draw call handling, so we should be inside a
    * render pass.
    */
   assert(cmd_buffer->state.pass);

   /* If this is the first time we update EZ state for this job we first check
    * if there is anything that requires disabling it completely for the entire
    * job (based on state that is not related to the current draw call and
    * pipeline state).
    */
   if (!job->decided_global_ez_enable) {
      job->decided_global_ez_enable = true;

      struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
      assert(state->subpass_idx < state->pass->subpass_count);
      struct v3dv_subpass *subpass = &state->pass->subpasses[state->subpass_idx];
      if (subpass->ds_attachment.attachment == VK_ATTACHMENT_UNUSED) {
         job->first_ez_state = V3D_EZ_DISABLED;
         job->ez_state = V3D_EZ_DISABLED;
         return false;
      }

      /* GFXH-1918: the early-z buffer may load incorrect depth values
       * if the frame has odd width or height.
       *
       * So we need to disable EZ in this case.
       */
      const struct v3dv_render_pass_attachment *ds_attachment =
         &state->pass->attachments[subpass->ds_attachment.attachment];

      const VkImageAspectFlags ds_aspects =
         vk_format_aspects(ds_attachment->desc.format);

      bool needs_depth_load =
         v3dv_cmd_buffer_check_needs_load(state,
                                          ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                          ds_attachment->first_subpass,
                                          ds_attachment->desc.loadOp,
                                          ds_attachment->last_subpass,
                                          ds_attachment->desc.storeOp);

      if (needs_depth_load) {
         struct v3dv_framebuffer *fb = state->framebuffer;

         if (!fb) {
            assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
            perf_debug("Loading depth aspect in a secondary command buffer "
                       "without framebuffer info disables early-z tests.\n");
            job->first_ez_state = V3D_EZ_DISABLED;
            job->ez_state = V3D_EZ_DISABLED;
            return false;
         }

         if (((fb->width % 2) != 0 || (fb->height % 2) != 0)) {
            perf_debug("Loading depth aspect for framebuffer with odd width "
                       "or height disables early-Z tests.\n");
            job->first_ez_state = V3D_EZ_DISABLED;
            job->ez_state = V3D_EZ_DISABLED;
            return false;
         }
      }
   }

   /* Otherwise, we can decide to selectively enable or disable EZ for draw
    * calls using the CFG_BITS packet based on the bound pipeline state.
    */
   bool disable_ez = false;
   bool incompatible_test = false;
   switch (pipeline->ez_state) {
   case V3D_EZ_UNDECIDED:
      /* If the pipeline didn't pick a direction but didn't disable, then go
       * along with the current EZ state. This allows EZ optimization for Z
       * func == EQUAL or NEVER.
       */
      break;

   case V3D_EZ_LT_LE:
   case V3D_EZ_GT_GE:
      /* If the pipeline picked a direction, then it needs to match the current
       * direction if we've decided on one.
       */
      if (job->ez_state == V3D_EZ_UNDECIDED) {
         job->ez_state = pipeline->ez_state;
      } else if (job->ez_state != pipeline->ez_state) {
         disable_ez = true;
         incompatible_test = true;
      }
      break;

   case V3D_EZ_DISABLED:
         disable_ez = true;
         incompatible_test = pipeline->incompatible_ez_test;
      break;
   }

   if (job->first_ez_state == V3D_EZ_UNDECIDED && !disable_ez) {
      assert(job->ez_state != V3D_EZ_DISABLED);
      job->first_ez_state = job->ez_state;
   }

   /* If we had to disable EZ because of an incompatible test direction and
    * and the pipeline writes depth then we need to disable EZ for the rest of
    * the frame.
    */
   if (incompatible_test && pipeline->z_updates_enable) {
      assert(disable_ez);
      job->ez_state = V3D_EZ_DISABLED;
   }

   if (!disable_ez)
      job->has_ez_draws = true;

   return !disable_ez;
}

void
v3dX(cmd_buffer_emit_configuration_bits)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   assert(pipeline);

   v3dv_cl_ensure_space_with_branch(&job->bcl, cl_packet_length(CFG_BITS));
   v3dv_return_if_oom(cmd_buffer, NULL);

   cl_emit_with_prepacked(&job->bcl, CFG_BITS, pipeline->cfg_bits, config) {
#if V3D_VERSION == 42
      bool enable_ez = job_update_ez_state(job, pipeline, cmd_buffer);
      config.early_z_enable = enable_ez;
      config.early_z_updates_enable = config.early_z_enable &&
         pipeline->z_updates_enable;
#endif
   }
}

void
v3dX(cmd_buffer_emit_occlusion_query)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   v3dv_cl_ensure_space_with_branch(&job->bcl,
                                    cl_packet_length(OCCLUSION_QUERY_COUNTER));
   v3dv_return_if_oom(cmd_buffer, NULL);

   cl_emit(&job->bcl, OCCLUSION_QUERY_COUNTER, counter) {
      if (cmd_buffer->state.query.active_query.bo) {
         counter.address =
            v3dv_cl_address(cmd_buffer->state.query.active_query.bo,
                            cmd_buffer->state.query.active_query.offset);
      }
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_OCCLUSION_QUERY;
}

static struct v3dv_job *
cmd_buffer_subpass_split_for_barrier(struct v3dv_cmd_buffer *cmd_buffer,
                                     bool is_bcl_barrier)
{
   assert(cmd_buffer->state.subpass_idx != -1);
   v3dv_cmd_buffer_finish_job(cmd_buffer);
   struct v3dv_job *job =
      v3dv_cmd_buffer_subpass_resume(cmd_buffer,
                                     cmd_buffer->state.subpass_idx);
   if (!job)
      return NULL;

   /* FIXME: we can do better than all barriers */
   job->serialize = V3DV_BARRIER_ALL;
   job->needs_bcl_sync = is_bcl_barrier;
   return job;
}

static void
cmd_buffer_copy_secondary_end_query_state(struct v3dv_cmd_buffer *primary,
                                          struct v3dv_cmd_buffer *secondary)
{
   struct v3dv_cmd_buffer_state *p_state = &primary->state;
   struct v3dv_cmd_buffer_state *s_state = &secondary->state;

   const uint32_t total_state_count =
      p_state->query.end.used_count + s_state->query.end.used_count;
   v3dv_cmd_buffer_ensure_array_state(primary,
                                      sizeof(struct v3dv_end_query_info),
                                      total_state_count,
                                      &p_state->query.end.alloc_count,
                                      (void **) &p_state->query.end.states);
   v3dv_return_if_oom(primary, NULL);

   for (uint32_t i = 0; i < s_state->query.end.used_count; i++) {
      const struct v3dv_end_query_info *s_qstate =
         &secondary->state.query.end.states[i];

      struct v3dv_end_query_info *p_qstate =
         &p_state->query.end.states[p_state->query.end.used_count++];

      p_qstate->pool = s_qstate->pool;
      p_qstate->query = s_qstate->query;
   }
}

void
v3dX(cmd_buffer_execute_inside_pass)(struct v3dv_cmd_buffer *primary,
                                     uint32_t cmd_buffer_count,
                                     const VkCommandBuffer *cmd_buffers)
{
   assert(primary->state.job);

   /* Typically we postpone applying binning syncs until we see a draw call
    * that may actually access proteted resources in the binning stage. However,
    * if the draw calls are recorded in a secondary command buffer and the
    * barriers were recorded in a primary command buffer, that won't work
    * and we will have to check if we need a binning sync when executing the
    * secondary.
    */
   struct v3dv_job *primary_job = primary->state.job;
   if (primary_job->serialize &&
       (primary->state.barrier.bcl_buffer_access ||
        primary->state.barrier.bcl_image_access)) {
      v3dv_cmd_buffer_consume_bcl_sync(primary, primary_job);
   }

   /* Emit occlusion query state if needed so the draw calls inside our
    * secondaries update the counters.
    */
   bool has_occlusion_query =
      primary->state.dirty & V3DV_CMD_DIRTY_OCCLUSION_QUERY;
   if (has_occlusion_query)
      v3dX(cmd_buffer_emit_occlusion_query)(primary);

   /* FIXME: if our primary job tiling doesn't enable MSSA but any of the
    * pipelines used by the secondaries do, we need to re-start the primary
    * job to enable MSAA. See cmd_buffer_restart_job_for_msaa_if_needed.
    */
   struct v3dv_barrier_state pending_barrier = { 0 };
   for (uint32_t i = 0; i < cmd_buffer_count; i++) {
      V3DV_FROM_HANDLE(v3dv_cmd_buffer, secondary, cmd_buffers[i]);

      assert(secondary->usage_flags &
             VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);

      list_for_each_entry(struct v3dv_job, secondary_job,
                          &secondary->jobs, list_link) {
         if (secondary_job->type == V3DV_JOB_TYPE_GPU_CL_SECONDARY) {
            /* If the job is a CL, then we branch to it from the primary BCL.
             * In this case the secondary's BCL is finished with a
             * RETURN_FROM_SUB_LIST command to return back to the primary BCL
             * once we are done executing it.
             */
            assert(v3dv_cl_offset(&secondary_job->rcl) == 0);
            assert(secondary_job->bcl.bo);

            /* Sanity check that secondary BCL ends with RETURN_FROM_SUB_LIST */
            STATIC_ASSERT(cl_packet_length(RETURN_FROM_SUB_LIST) == 1);
            assert(v3dv_cl_offset(&secondary_job->bcl) >= 1);
            assert(*(((uint8_t *)secondary_job->bcl.next) - 1) ==
                   V3DX(RETURN_FROM_SUB_LIST_opcode));

            /* If this secondary has any barriers (or we had any pending barrier
             * to apply), then we can't just branch to it from the primary, we
             * need to split the primary to create a new job that can consume
             * the barriers first.
             *
             * FIXME: in this case, maybe just copy the secondary BCL without
             * the RETURN_FROM_SUB_LIST into the primary job to skip the
             * branch?
             */
            primary_job = primary->state.job;
            if (!primary_job || secondary_job->serialize ||
                pending_barrier.dst_mask) {
               const bool needs_bcl_barrier =
                  secondary_job->needs_bcl_sync ||
                  pending_barrier.bcl_buffer_access ||
                  pending_barrier.bcl_image_access;

               primary_job =
                  cmd_buffer_subpass_split_for_barrier(primary,
                                                       needs_bcl_barrier);
               v3dv_return_if_oom(primary, NULL);

               /* Since we have created a new primary we need to re-emit
                * occlusion query state.
                */
               if (has_occlusion_query)
                  v3dX(cmd_buffer_emit_occlusion_query)(primary);
            }

            /* Make sure our primary job has all required BO references */
            set_foreach(secondary_job->bos, entry) {
               struct v3dv_bo *bo = (struct v3dv_bo *)entry->key;
               v3dv_job_add_bo(primary_job, bo);
            }

            /* Emit required branch instructions. We expect each of these
             * to end with a corresponding 'return from sub list' item.
             */
            list_for_each_entry(struct v3dv_bo, bcl_bo,
                                &secondary_job->bcl.bo_list, list_link) {
               v3dv_cl_ensure_space_with_branch(&primary_job->bcl,
                                                cl_packet_length(BRANCH_TO_SUB_LIST));
               v3dv_return_if_oom(primary, NULL);
               cl_emit(&primary_job->bcl, BRANCH_TO_SUB_LIST, branch) {
                  branch.address = v3dv_cl_address(bcl_bo, 0);
               }
            }

            if (!secondary_job->can_use_double_buffer) {
               primary_job->can_use_double_buffer = false;
            } else {
               primary_job->double_buffer_score.geom +=
                  secondary_job->double_buffer_score.geom;
               primary_job->double_buffer_score.render +=
                  secondary_job->double_buffer_score.render;
            }
            primary_job->tmu_dirty_rcl |= secondary_job->tmu_dirty_rcl;
         } else {
            /* This is a regular job (CPU or GPU), so just finish the current
             * primary job (if any) and then add the secondary job to the
             * primary's job list right after it.
             */
            v3dv_cmd_buffer_finish_job(primary);
            v3dv_job_clone_in_cmd_buffer(secondary_job, primary);
            if (pending_barrier.dst_mask) {
               /* FIXME: do the same we do for primaries and only choose the
                * relevant src masks.
                */
               secondary_job->serialize = pending_barrier.src_mask_graphics |
                                          pending_barrier.src_mask_transfer |
                                          pending_barrier.src_mask_compute;
               if (pending_barrier.bcl_buffer_access ||
                   pending_barrier.bcl_image_access) {
                  secondary_job->needs_bcl_sync = true;
               }
            }
         }

         memset(&pending_barrier, 0, sizeof(pending_barrier));
      }

      /* If the secondary has recorded any vkCmdEndQuery commands, we need to
       * copy this state to the primary so it is processed properly when the
       * current primary job is finished.
       */
      cmd_buffer_copy_secondary_end_query_state(primary, secondary);

      /* If this secondary had any pending barrier state we will need that
       * barrier state consumed with whatever comes next in the primary.
       */
      assert(secondary->state.barrier.dst_mask ||
             (!secondary->state.barrier.bcl_buffer_access &&
              !secondary->state.barrier.bcl_image_access));

      pending_barrier = secondary->state.barrier;
   }

   if (pending_barrier.dst_mask) {
      v3dv_cmd_buffer_merge_barrier_state(&primary->state.barrier,
                                          &pending_barrier);
   }
}

static void
emit_gs_shader_state_record(struct v3dv_job *job,
                            struct v3dv_bo *assembly_bo,
                            struct v3dv_shader_variant *gs_bin,
                            struct v3dv_cl_reloc gs_bin_uniforms,
                            struct v3dv_shader_variant *gs,
                            struct v3dv_cl_reloc gs_render_uniforms)
{
   cl_emit(&job->indirect, GEOMETRY_SHADER_STATE_RECORD, shader) {
      shader.geometry_bin_mode_shader_code_address =
         v3dv_cl_address(assembly_bo, gs_bin->assembly_offset);
      shader.geometry_bin_mode_shader_4_way_threadable =
         gs_bin->prog_data.gs->base.threads == 4;
      shader.geometry_bin_mode_shader_start_in_final_thread_section =
         gs_bin->prog_data.gs->base.single_seg;
#if V3D_VERSION <= 42
      shader.geometry_bin_mode_shader_propagate_nans = true;
#endif
      shader.geometry_bin_mode_shader_uniforms_address =
         gs_bin_uniforms;

      shader.geometry_render_mode_shader_code_address =
         v3dv_cl_address(assembly_bo, gs->assembly_offset);
      shader.geometry_render_mode_shader_4_way_threadable =
         gs->prog_data.gs->base.threads == 4;
      shader.geometry_render_mode_shader_start_in_final_thread_section =
         gs->prog_data.gs->base.single_seg;
#if V3D_VERSION <= 42
      shader.geometry_render_mode_shader_propagate_nans = true;
#endif
      shader.geometry_render_mode_shader_uniforms_address =
         gs_render_uniforms;
   }
}

static uint8_t
v3d_gs_output_primitive(enum mesa_prim prim_type)
{
    switch (prim_type) {
    case MESA_PRIM_POINTS:
        return GEOMETRY_SHADER_POINTS;
    case MESA_PRIM_LINE_STRIP:
        return GEOMETRY_SHADER_LINE_STRIP;
    case MESA_PRIM_TRIANGLE_STRIP:
        return GEOMETRY_SHADER_TRI_STRIP;
    default:
        unreachable("Unsupported primitive type");
    }
}

static void
emit_tes_gs_common_params(struct v3dv_job *job,
                          uint8_t gs_out_prim_type,
                          uint8_t gs_num_invocations)
{
   cl_emit(&job->indirect, TESSELLATION_GEOMETRY_COMMON_PARAMS, shader) {
      shader.tessellation_type = TESSELLATION_TYPE_TRIANGLE;
      shader.tessellation_point_mode = false;
      shader.tessellation_edge_spacing = TESSELLATION_EDGE_SPACING_EVEN;
      shader.tessellation_clockwise = true;
      shader.tessellation_invocations = 1;

      shader.geometry_shader_output_format =
         v3d_gs_output_primitive(gs_out_prim_type);
      shader.geometry_shader_instances = gs_num_invocations & 0x1F;
   }
}

static uint8_t
simd_width_to_gs_pack_mode(uint32_t width)
{
   switch (width) {
   case 16:
      return V3D_PACK_MODE_16_WAY;
   case 8:
      return V3D_PACK_MODE_8_WAY;
   case 4:
      return V3D_PACK_MODE_4_WAY;
   case 1:
      return V3D_PACK_MODE_1_WAY;
   default:
      unreachable("Invalid SIMD width");
   };
}

static void
emit_tes_gs_shader_params(struct v3dv_job *job,
                          uint32_t gs_simd,
                          uint32_t gs_vpm_output_size,
                          uint32_t gs_max_vpm_input_size_per_batch)
{
   cl_emit(&job->indirect, TESSELLATION_GEOMETRY_SHADER_PARAMS, shader) {
      shader.tcs_batch_flush_mode = V3D_TCS_FLUSH_MODE_FULLY_PACKED;
      shader.per_patch_data_column_depth = 1;
      shader.tcs_output_segment_size_in_sectors = 1;
      shader.tcs_output_segment_pack_mode = V3D_PACK_MODE_16_WAY;
      shader.tes_output_segment_size_in_sectors = 1;
      shader.tes_output_segment_pack_mode = V3D_PACK_MODE_16_WAY;
      shader.gs_output_segment_size_in_sectors = gs_vpm_output_size;
      shader.gs_output_segment_pack_mode =
         simd_width_to_gs_pack_mode(gs_simd);
      shader.tbg_max_patches_per_tcs_batch = 1;
      shader.tbg_max_extra_vertex_segs_for_patches_after_first = 0;
      shader.tbg_min_tcs_output_segments_required_in_play = 1;
      shader.tbg_min_per_patch_data_segments_required_in_play = 1;
      shader.tpg_max_patches_per_tes_batch = 1;
      shader.tpg_max_vertex_segments_per_tes_batch = 0;
      shader.tpg_max_tcs_output_segments_per_tes_batch = 1;
      shader.tpg_min_tes_output_segments_required_in_play = 1;
      shader.gbg_max_tes_output_vertex_segments_per_gs_batch =
         gs_max_vpm_input_size_per_batch;
      shader.gbg_min_gs_output_segments_required_in_play = 1;
   }
}

void
v3dX(cmd_buffer_emit_gl_shader_state)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   struct v3dv_pipeline *pipeline = state->gfx.pipeline;
   assert(pipeline);

   struct v3dv_shader_variant *vs_variant =
      pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX];
   struct v3d_vs_prog_data *prog_data_vs = vs_variant->prog_data.vs;

   struct v3dv_shader_variant *vs_bin_variant =
      pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX_BIN];
   struct v3d_vs_prog_data *prog_data_vs_bin = vs_bin_variant->prog_data.vs;

   struct v3dv_shader_variant *fs_variant =
      pipeline->shared_data->variants[BROADCOM_SHADER_FRAGMENT];
   struct v3d_fs_prog_data *prog_data_fs = fs_variant->prog_data.fs;

   struct v3dv_shader_variant *gs_variant = NULL;
   struct v3dv_shader_variant *gs_bin_variant = NULL;
   struct v3d_gs_prog_data *prog_data_gs = NULL;
   struct v3d_gs_prog_data *prog_data_gs_bin = NULL;
   if (pipeline->has_gs) {
      gs_variant =
         pipeline->shared_data->variants[BROADCOM_SHADER_GEOMETRY];
      prog_data_gs = gs_variant->prog_data.gs;

      gs_bin_variant =
         pipeline->shared_data->variants[BROADCOM_SHADER_GEOMETRY_BIN];
      prog_data_gs_bin = gs_bin_variant->prog_data.gs;
   }

   /* Update the cache dirty flag based on the shader progs data */
   job->tmu_dirty_rcl |= prog_data_vs_bin->base.tmu_dirty_rcl;
   job->tmu_dirty_rcl |= prog_data_vs->base.tmu_dirty_rcl;
   job->tmu_dirty_rcl |= prog_data_fs->base.tmu_dirty_rcl;
   if (pipeline->has_gs) {
      job->tmu_dirty_rcl |= prog_data_gs_bin->base.tmu_dirty_rcl;
      job->tmu_dirty_rcl |= prog_data_gs->base.tmu_dirty_rcl;
   }

   /* See GFXH-930 workaround below */
   uint32_t num_elements_to_emit = MAX2(pipeline->va_count, 1);

   uint32_t shader_state_record_length =
      cl_packet_length(GL_SHADER_STATE_RECORD);
   if (pipeline->has_gs) {
      shader_state_record_length +=
         cl_packet_length(GEOMETRY_SHADER_STATE_RECORD) +
         cl_packet_length(TESSELLATION_GEOMETRY_COMMON_PARAMS) +
         2 * cl_packet_length(TESSELLATION_GEOMETRY_SHADER_PARAMS);
   }

   uint32_t shader_rec_offset =
      v3dv_cl_ensure_space(&job->indirect,
                           shader_state_record_length +
                           num_elements_to_emit *
                           cl_packet_length(GL_SHADER_STATE_ATTRIBUTE_RECORD),
                           32);
   v3dv_return_if_oom(cmd_buffer, NULL);

   struct v3dv_bo *assembly_bo = pipeline->shared_data->assembly_bo;

   if (pipeline->has_gs) {
      emit_gs_shader_state_record(job,
                                  assembly_bo,
                                  gs_bin_variant,
                                  cmd_buffer->state.uniforms.gs_bin,
                                  gs_variant,
                                  cmd_buffer->state.uniforms.gs);

      emit_tes_gs_common_params(job,
                                prog_data_gs->out_prim_type,
                                prog_data_gs->num_invocations);

      emit_tes_gs_shader_params(job,
                                pipeline->vpm_cfg_bin.gs_width,
                                pipeline->vpm_cfg_bin.Gd,
                                pipeline->vpm_cfg_bin.Gv);

      emit_tes_gs_shader_params(job,
                                pipeline->vpm_cfg.gs_width,
                                pipeline->vpm_cfg.Gd,
                                pipeline->vpm_cfg.Gv);
   }

#if V3D_VERSION == 42
   struct v3dv_bo *default_attribute_values =
      pipeline->default_attribute_values != NULL ?
      pipeline->default_attribute_values :
      pipeline->device->default_attribute_float;
#endif

   cl_emit_with_prepacked(&job->indirect, GL_SHADER_STATE_RECORD,
                          pipeline->shader_state_record, shader) {

      /* FIXME: we are setting this values here and during the
       * prepacking. This is because both cl_emit_with_prepacked and v3dvx_pack
       * asserts for minimum values of these. It would be good to get
       * v3dvx_pack to assert on the final value if possible
       */
      shader.min_coord_shader_input_segments_required_in_play =
         pipeline->vpm_cfg_bin.As;
      shader.min_vertex_shader_input_segments_required_in_play =
         pipeline->vpm_cfg.As;

      shader.coordinate_shader_code_address =
         v3dv_cl_address(assembly_bo, vs_bin_variant->assembly_offset);
      shader.vertex_shader_code_address =
         v3dv_cl_address(assembly_bo, vs_variant->assembly_offset);
      shader.fragment_shader_code_address =
         v3dv_cl_address(assembly_bo, fs_variant->assembly_offset);

      shader.coordinate_shader_uniforms_address = cmd_buffer->state.uniforms.vs_bin;
      shader.vertex_shader_uniforms_address = cmd_buffer->state.uniforms.vs;
      shader.fragment_shader_uniforms_address = cmd_buffer->state.uniforms.fs;

#if V3D_VERSION == 42
      shader.address_of_default_attribute_values =
         v3dv_cl_address(default_attribute_values, 0);
#endif

      shader.any_shader_reads_hardware_written_primitive_id =
         (pipeline->has_gs && prog_data_gs->uses_pid) || prog_data_fs->uses_pid;
      shader.insert_primitive_id_as_first_varying_to_fragment_shader =
         !pipeline->has_gs && prog_data_fs->uses_pid;
   }

   /* Upload vertex element attributes (SHADER_STATE_ATTRIBUTE_RECORD) */
   bool cs_loaded_any = false;
   const bool cs_uses_builtins = prog_data_vs_bin->uses_iid ||
                                 prog_data_vs_bin->uses_biid ||
                                 prog_data_vs_bin->uses_vid;
   const uint32_t packet_length =
      cl_packet_length(GL_SHADER_STATE_ATTRIBUTE_RECORD);

   uint32_t emitted_va_count = 0;
   for (uint32_t i = 0; emitted_va_count < pipeline->va_count; i++) {
      assert(i < MAX_VERTEX_ATTRIBS);

      if (pipeline->va[i].vk_format == VK_FORMAT_UNDEFINED)
         continue;

      const uint32_t binding = pipeline->va[i].binding;

      /* We store each vertex attribute in the array using its driver location
       * as index.
       */
      const uint32_t location = i;

      struct v3dv_vertex_binding *c_vb = &cmd_buffer->state.vertex_bindings[binding];

      cl_emit_with_prepacked(&job->indirect, GL_SHADER_STATE_ATTRIBUTE_RECORD,
                             &pipeline->vertex_attrs[i * packet_length], attr) {

         assert(c_vb->buffer->mem->bo);
         attr.address = v3dv_cl_address(c_vb->buffer->mem->bo,
                                        c_vb->buffer->mem_offset +
                                        pipeline->va[i].offset +
                                        c_vb->offset);

         attr.number_of_values_read_by_coordinate_shader =
            prog_data_vs_bin->vattr_sizes[location];
         attr.number_of_values_read_by_vertex_shader =
            prog_data_vs->vattr_sizes[location];

         /* GFXH-930: At least one attribute must be enabled and read by CS
          * and VS.  If we have attributes being consumed by the VS but not
          * the CS, then set up a dummy load of the last attribute into the
          * CS's VPM inputs.  (Since CS is just dead-code-elimination compared
          * to VS, we can't have CS loading but not VS).
          *
          * GFXH-1602: first attribute must be active if using builtins.
          */
         if (prog_data_vs_bin->vattr_sizes[location])
            cs_loaded_any = true;

         if (i == 0 && cs_uses_builtins && !cs_loaded_any) {
            attr.number_of_values_read_by_coordinate_shader = 1;
            cs_loaded_any = true;
         } else if (i == pipeline->va_count - 1 && !cs_loaded_any) {
            attr.number_of_values_read_by_coordinate_shader = 1;
            cs_loaded_any = true;
         }

         attr.maximum_index = 0xffffff;
      }

      emitted_va_count++;
   }

   if (pipeline->va_count == 0) {
      /* GFXH-930: At least one attribute must be enabled and read
       * by CS and VS.  If we have no attributes being consumed by
       * the shader, set up a dummy to be loaded into the VPM.
       */
      cl_emit(&job->indirect, GL_SHADER_STATE_ATTRIBUTE_RECORD, attr) {
         /* Valid address of data whose value will be unused. */
         attr.address = v3dv_cl_address(job->indirect.bo, 0);

         attr.type = ATTRIBUTE_FLOAT;
         attr.stride = 0;
         attr.vec_size = 1;

         attr.number_of_values_read_by_coordinate_shader = 1;
         attr.number_of_values_read_by_vertex_shader = 1;
      }
   }

   if (cmd_buffer->state.dirty & V3DV_CMD_DIRTY_PIPELINE) {
      v3dv_cl_ensure_space_with_branch(&job->bcl,
                                       sizeof(pipeline->vcm_cache_size));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit_prepacked(&job->bcl, &pipeline->vcm_cache_size);
   }

   v3dv_cl_ensure_space_with_branch(&job->bcl,
                                    cl_packet_length(GL_SHADER_STATE));
   v3dv_return_if_oom(cmd_buffer, NULL);

   if (pipeline->has_gs) {
      cl_emit(&job->bcl, GL_SHADER_STATE_INCLUDING_GS, state) {
         state.address = v3dv_cl_address(job->indirect.bo, shader_rec_offset);
         state.number_of_attribute_arrays = num_elements_to_emit;
      }
   } else {
      cl_emit(&job->bcl, GL_SHADER_STATE, state) {
         state.address = v3dv_cl_address(job->indirect.bo, shader_rec_offset);
         state.number_of_attribute_arrays = num_elements_to_emit;
      }
   }

   /* Clearing push constants and descriptor sets for all stages is not quite
    * correct (some shader stages may not be used at all or they may not be
    * consuming push constants), however this is not relevant because if we
    * bind a different pipeline we always have to rebuild the uniform streams.
    */
   cmd_buffer->state.dirty &= ~(V3DV_CMD_DIRTY_VERTEX_BUFFER |
                                V3DV_CMD_DIRTY_DESCRIPTOR_SETS |
                                V3DV_CMD_DIRTY_PUSH_CONSTANTS);
   cmd_buffer->state.dirty_descriptor_stages &= ~VK_SHADER_STAGE_ALL_GRAPHICS;
   cmd_buffer->state.dirty_push_constants_stages &= ~VK_SHADER_STAGE_ALL_GRAPHICS;
}

void
v3dX(cmd_buffer_emit_draw)(struct v3dv_cmd_buffer *cmd_buffer,
                           struct v3dv_draw_info *info)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   struct v3dv_pipeline *pipeline = state->gfx.pipeline;

   assert(pipeline);

   uint32_t hw_prim_type = v3d_hw_prim_type(pipeline->topology);

   if (info->first_instance > 0) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(BASE_VERTEX_BASE_INSTANCE));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, BASE_VERTEX_BASE_INSTANCE, base) {
         base.base_instance = info->first_instance;
         base.base_vertex = 0;
      }
   }

   if (info->instance_count > 1) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(VERTEX_ARRAY_INSTANCED_PRIMS));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, VERTEX_ARRAY_INSTANCED_PRIMS, prim) {
         prim.mode = hw_prim_type;
         prim.index_of_first_vertex = info->first_vertex;
         prim.number_of_instances = info->instance_count;
         prim.instance_length = info->vertex_count;
      }
   } else {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(VERTEX_ARRAY_PRIMS));
      v3dv_return_if_oom(cmd_buffer, NULL);
      cl_emit(&job->bcl, VERTEX_ARRAY_PRIMS, prim) {
         prim.mode = hw_prim_type;
         prim.length = info->vertex_count;
         prim.index_of_first_vertex = info->first_vertex;
      }
   }
}

void
v3dX(cmd_buffer_emit_index_buffer)(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   /* We flag all state as dirty when we create a new job so make sure we
    * have a valid index buffer before attempting to emit state for it.
    */
   struct v3dv_buffer *ibuffer =
      v3dv_buffer_from_handle(cmd_buffer->state.index_buffer.buffer);
   if (ibuffer) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(INDEX_BUFFER_SETUP));
      v3dv_return_if_oom(cmd_buffer, NULL);

      const uint32_t offset = cmd_buffer->state.index_buffer.offset;
      cl_emit(&job->bcl, INDEX_BUFFER_SETUP, ib) {
         ib.address = v3dv_cl_address(ibuffer->mem->bo,
                                      ibuffer->mem_offset + offset);
         ib.size = ibuffer->mem->bo->size;
      }
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_INDEX_BUFFER;
}

void
v3dX(cmd_buffer_emit_draw_indexed)(struct v3dv_cmd_buffer *cmd_buffer,
                                   uint32_t indexCount,
                                   uint32_t instanceCount,
                                   uint32_t firstIndex,
                                   int32_t vertexOffset,
                                   uint32_t firstInstance)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   const struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   uint32_t hw_prim_type = v3d_hw_prim_type(pipeline->topology);
   uint8_t index_type = ffs(cmd_buffer->state.index_buffer.index_size) - 1;
   uint32_t index_offset = firstIndex * cmd_buffer->state.index_buffer.index_size;

   if (vertexOffset != 0 || firstInstance != 0) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(BASE_VERTEX_BASE_INSTANCE));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, BASE_VERTEX_BASE_INSTANCE, base) {
         base.base_instance = firstInstance;
         base.base_vertex = vertexOffset;
      }
   }

   if (instanceCount == 1) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(INDEXED_PRIM_LIST));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, INDEXED_PRIM_LIST, prim) {
         prim.index_type = index_type;
         prim.length = indexCount;
         prim.index_offset = index_offset;
         prim.mode = hw_prim_type;
         prim.enable_primitive_restarts = pipeline->primitive_restart;
      }
   } else if (instanceCount > 1) {
      v3dv_cl_ensure_space_with_branch(
         &job->bcl, cl_packet_length(INDEXED_INSTANCED_PRIM_LIST));
      v3dv_return_if_oom(cmd_buffer, NULL);

      cl_emit(&job->bcl, INDEXED_INSTANCED_PRIM_LIST, prim) {
         prim.index_type = index_type;
         prim.index_offset = index_offset;
         prim.mode = hw_prim_type;
         prim.enable_primitive_restarts = pipeline->primitive_restart;
         prim.number_of_instances = instanceCount;
         prim.instance_length = indexCount;
      }
   }
}

void
v3dX(cmd_buffer_emit_draw_indirect)(struct v3dv_cmd_buffer *cmd_buffer,
                                    struct v3dv_buffer *buffer,
                                    VkDeviceSize offset,
                                    uint32_t drawCount,
                                    uint32_t stride)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   const struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   uint32_t hw_prim_type = v3d_hw_prim_type(pipeline->topology);

   v3dv_cl_ensure_space_with_branch(
      &job->bcl, cl_packet_length(INDIRECT_VERTEX_ARRAY_INSTANCED_PRIMS));
   v3dv_return_if_oom(cmd_buffer, NULL);

   cl_emit(&job->bcl, INDIRECT_VERTEX_ARRAY_INSTANCED_PRIMS, prim) {
      prim.mode = hw_prim_type;
      prim.number_of_draw_indirect_array_records = drawCount;
      prim.stride_in_multiples_of_4_bytes = stride >> 2;
      prim.address = v3dv_cl_address(buffer->mem->bo,
                                     buffer->mem_offset + offset);
   }
}

void
v3dX(cmd_buffer_emit_indexed_indirect)(struct v3dv_cmd_buffer *cmd_buffer,
                                       struct v3dv_buffer *buffer,
                                       VkDeviceSize offset,
                                       uint32_t drawCount,
                                       uint32_t stride)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   const struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   uint32_t hw_prim_type = v3d_hw_prim_type(pipeline->topology);
   uint8_t index_type = ffs(cmd_buffer->state.index_buffer.index_size) - 1;

   v3dv_cl_ensure_space_with_branch(
      &job->bcl, cl_packet_length(INDIRECT_INDEXED_INSTANCED_PRIM_LIST));
   v3dv_return_if_oom(cmd_buffer, NULL);

   cl_emit(&job->bcl, INDIRECT_INDEXED_INSTANCED_PRIM_LIST, prim) {
      prim.index_type = index_type;
      prim.mode = hw_prim_type;
      prim.enable_primitive_restarts = pipeline->primitive_restart;
      prim.number_of_draw_indirect_indexed_records = drawCount;
      prim.stride_in_multiples_of_4_bytes = stride >> 2;
      prim.address = v3dv_cl_address(buffer->mem->bo,
                                     buffer->mem_offset + offset);
   }
}
