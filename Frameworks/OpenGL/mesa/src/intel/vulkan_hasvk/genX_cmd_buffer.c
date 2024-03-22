/*
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>

#include "anv_private.h"
#include "anv_measure.h"
#include "vk_format.h"
#include "vk_render_pass.h"
#include "vk_util.h"
#include "util/fast_idiv_by_const.h"

#include "common/intel_l3_config.h"
#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"
#include "genxml/genX_rt_pack.h"
#include "common/intel_guardband.h"
#include "compiler/brw_prim.h"

#include "nir/nir_xfb_info.h"

#include "ds/intel_tracepoints.h"

/* We reserve :
 *    - GPR 14 for secondary command buffer returns
 *    - GPR 15 for conditional rendering
 */
#define MI_BUILDER_NUM_ALLOC_GPRS 14
#define __gen_get_batch_dwords anv_batch_emit_dwords
#define __gen_address_offset anv_address_add
#define __gen_get_batch_address(b, a) anv_batch_address(b, a)
#include "common/mi_builder.h"

static void genX(flush_pipeline_select)(struct anv_cmd_buffer *cmd_buffer,
                                        uint32_t pipeline);

static enum anv_pipe_bits
convert_pc_to_bits(struct GENX(PIPE_CONTROL) *pc) {
   enum anv_pipe_bits bits = 0;
   bits |= (pc->DepthCacheFlushEnable) ?  ANV_PIPE_DEPTH_CACHE_FLUSH_BIT : 0;
   bits |= (pc->DCFlushEnable) ?  ANV_PIPE_DATA_CACHE_FLUSH_BIT : 0;
   bits |= (pc->RenderTargetCacheFlushEnable) ?  ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT : 0;
   bits |= (pc->VFCacheInvalidationEnable) ?  ANV_PIPE_VF_CACHE_INVALIDATE_BIT : 0;
   bits |= (pc->StateCacheInvalidationEnable) ?  ANV_PIPE_STATE_CACHE_INVALIDATE_BIT : 0;
   bits |= (pc->ConstantCacheInvalidationEnable) ?  ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT : 0;
   bits |= (pc->TextureCacheInvalidationEnable) ?  ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT : 0;
   bits |= (pc->InstructionCacheInvalidateEnable) ?  ANV_PIPE_INSTRUCTION_CACHE_INVALIDATE_BIT : 0;
   bits |= (pc->StallAtPixelScoreboard) ?  ANV_PIPE_STALL_AT_SCOREBOARD_BIT : 0;
   bits |= (pc->DepthStallEnable) ?  ANV_PIPE_DEPTH_STALL_BIT : 0;
   bits |= (pc->CommandStreamerStallEnable) ?  ANV_PIPE_CS_STALL_BIT : 0;
   return bits;
}

#define anv_debug_dump_pc(pc) \
   if (INTEL_DEBUG(DEBUG_PIPE_CONTROL)) { \
      fputs("pc: emit PC=( ", stderr); \
      anv_dump_pipe_bits(convert_pc_to_bits(&(pc))); \
      fprintf(stderr, ") reason: %s\n", __func__); \
   }

static bool
is_render_queue_cmd_buffer(const struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_queue_family *queue_family = cmd_buffer->queue_family;
   return (queue_family->queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
}

void
genX(cmd_buffer_emit_state_base_address)(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_device *device = cmd_buffer->device;
   uint32_t mocs = isl_mocs(&device->isl_dev, 0, false);

   /* If we are emitting a new state base address we probably need to re-emit
    * binding tables.
    */
   cmd_buffer->state.descriptors_dirty |= ~0;

   /* Emit a render target cache flush.
    *
    * This isn't documented anywhere in the PRM.  However, it seems to be
    * necessary prior to changing the surface state base address.  Without
    * this, we get GPU hangs when using multi-level command buffers which
    * clear depth, reset state base address, and then go render stuff.
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.DCFlushEnable = true;
      pc.RenderTargetCacheFlushEnable = true;
      pc.CommandStreamerStallEnable = true;
      anv_debug_dump_pc(pc);
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(STATE_BASE_ADDRESS), sba) {
      sba.GeneralStateBaseAddress = (struct anv_address) { NULL, 0 };
      sba.GeneralStateMOCS = mocs;
      sba.GeneralStateBaseAddressModifyEnable = true;

      sba.StatelessDataPortAccessMOCS = mocs;

      sba.SurfaceStateBaseAddress =
         anv_cmd_buffer_surface_base_address(cmd_buffer);
      sba.SurfaceStateMOCS = mocs;
      sba.SurfaceStateBaseAddressModifyEnable = true;

      sba.DynamicStateBaseAddress =
         (struct anv_address) { device->dynamic_state_pool.block_pool.bo, 0 };
      sba.DynamicStateMOCS = mocs;
      sba.DynamicStateBaseAddressModifyEnable = true;

      sba.IndirectObjectBaseAddress = (struct anv_address) { NULL, 0 };
      sba.IndirectObjectMOCS = mocs;
      sba.IndirectObjectBaseAddressModifyEnable = true;

      sba.InstructionBaseAddress =
         (struct anv_address) { device->instruction_state_pool.block_pool.bo, 0 };
      sba.InstructionMOCS = mocs;
      sba.InstructionBaseAddressModifyEnable = true;

#  if (GFX_VER >= 8)
      /* Broadwell requires that we specify a buffer size for a bunch of
       * these fields.  However, since we will be growing the BO's live, we
       * just set them all to the maximum.
       */
      sba.GeneralStateBufferSize       = 0xfffff;
      sba.IndirectObjectBufferSize     = 0xfffff;
      if (anv_use_relocations(device->physical)) {
         sba.DynamicStateBufferSize    = 0xfffff;
         sba.InstructionBufferSize     = 0xfffff;
      } else {
         /* With softpin, we use fixed addresses so we actually know how big
          * our base addresses are.
          */
         sba.DynamicStateBufferSize    = DYNAMIC_STATE_POOL_SIZE / 4096;
         sba.InstructionBufferSize     = INSTRUCTION_STATE_POOL_SIZE / 4096;
      }
      sba.GeneralStateBufferSizeModifyEnable    = true;
      sba.IndirectObjectBufferSizeModifyEnable  = true;
      sba.DynamicStateBufferSizeModifyEnable    = true;
      sba.InstructionBuffersizeModifyEnable     = true;
#  else
      /* On gfx7, we have upper bounds instead.  According to the docs,
       * setting an upper bound of zero means that no bounds checking is
       * performed so, in theory, we should be able to leave them zero.
       * However, border color is broken and the GPU bounds-checks anyway.
       * To avoid this and other potential problems, we may as well set it
       * for everything.
       */
      sba.GeneralStateAccessUpperBound =
         (struct anv_address) { .bo = NULL, .offset = 0xfffff000 };
      sba.GeneralStateAccessUpperBoundModifyEnable = true;
      sba.DynamicStateAccessUpperBound =
         (struct anv_address) { .bo = NULL, .offset = 0xfffff000 };
      sba.DynamicStateAccessUpperBoundModifyEnable = true;
      sba.InstructionAccessUpperBound =
         (struct anv_address) { .bo = NULL, .offset = 0xfffff000 };
      sba.InstructionAccessUpperBoundModifyEnable = true;
#  endif
   }

   /* After re-setting the surface state base address, we have to do some
    * cache flushing so that the sampler engine will pick up the new
    * SURFACE_STATE objects and binding tables. From the Broadwell PRM,
    * Shared Function > 3D Sampler > State > State Caching (page 96):
    *
    *    Coherency with system memory in the state cache, like the texture
    *    cache is handled partially by software. It is expected that the
    *    command stream or shader will issue Cache Flush operation or
    *    Cache_Flush sampler message to ensure that the L1 cache remains
    *    coherent with system memory.
    *
    *    [...]
    *
    *    Whenever the value of the Dynamic_State_Base_Addr,
    *    Surface_State_Base_Addr are altered, the L1 state cache must be
    *    invalidated to ensure the new surface or sampler state is fetched
    *    from system memory.
    *
    * The PIPE_CONTROL command has a "State Cache Invalidation Enable" bit
    * which, according the PIPE_CONTROL instruction documentation in the
    * Broadwell PRM:
    *
    *    Setting this bit is independent of any other bit in this packet.
    *    This bit controls the invalidation of the L1 and L2 state caches
    *    at the top of the pipe i.e. at the parsing time.
    *
    * Unfortunately, experimentation seems to indicate that state cache
    * invalidation through a PIPE_CONTROL does nothing whatsoever in
    * regards to surface state and binding tables.  In stead, it seems that
    * invalidating the texture cache is what is actually needed.
    *
    * XXX:  As far as we have been able to determine through
    * experimentation, shows that flush the texture cache appears to be
    * sufficient.  The theory here is that all of the sampling/rendering
    * units cache the binding table in the texture cache.  However, we have
    * yet to be able to actually confirm this.
    *
    * Wa_14013910100:
    *
    *  "DG2 128/256/512-A/B: S/W must program STATE_BASE_ADDRESS command twice
    *   or program pipe control with Instruction cache invalidate post
    *   STATE_BASE_ADDRESS command"
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.TextureCacheInvalidationEnable = true;
      pc.ConstantCacheInvalidationEnable = true;
      pc.StateCacheInvalidationEnable = true;
      anv_debug_dump_pc(pc);
   }
}

static void
add_surface_reloc(struct anv_cmd_buffer *cmd_buffer,
                  struct anv_state state, struct anv_address addr)
{
   VkResult result;

   if (anv_use_relocations(cmd_buffer->device->physical)) {
      const struct isl_device *isl_dev = &cmd_buffer->device->isl_dev;
      result = anv_reloc_list_add(&cmd_buffer->surface_relocs,
                                  &cmd_buffer->vk.pool->alloc,
                                  state.offset + isl_dev->ss.addr_offset,
                                  addr.bo, addr.offset, NULL);
   } else {
      result = anv_reloc_list_add_bo(&cmd_buffer->surface_relocs,
                                     &cmd_buffer->vk.pool->alloc,
                                     addr.bo);
   }

   if (unlikely(result != VK_SUCCESS))
      anv_batch_set_error(&cmd_buffer->batch, result);
}

static void
add_surface_state_relocs(struct anv_cmd_buffer *cmd_buffer,
                         struct anv_surface_state state)
{
   const struct isl_device *isl_dev = &cmd_buffer->device->isl_dev;

   assert(!anv_address_is_null(state.address));
   add_surface_reloc(cmd_buffer, state.state, state.address);

   if (!anv_address_is_null(state.aux_address)) {
      VkResult result =
         anv_reloc_list_add(&cmd_buffer->surface_relocs,
                            &cmd_buffer->vk.pool->alloc,
                            state.state.offset + isl_dev->ss.aux_addr_offset,
                            state.aux_address.bo,
                            state.aux_address.offset,
                            NULL);
      if (result != VK_SUCCESS)
         anv_batch_set_error(&cmd_buffer->batch, result);
   }

   if (!anv_address_is_null(state.clear_address)) {
      VkResult result =
         anv_reloc_list_add(&cmd_buffer->surface_relocs,
                            &cmd_buffer->vk.pool->alloc,
                            state.state.offset +
                            isl_dev->ss.clear_color_state_offset,
                            state.clear_address.bo,
                            state.clear_address.offset,
                            NULL);
      if (result != VK_SUCCESS)
         anv_batch_set_error(&cmd_buffer->batch, result);
   }
}

static bool
isl_color_value_requires_conversion(union isl_color_value color,
                                    const struct isl_surf *surf,
                                    const struct isl_view *view)
{
   if (surf->format == view->format && isl_swizzle_is_identity(view->swizzle))
      return false;

   uint32_t surf_pack[4] = { 0, 0, 0, 0 };
   isl_color_value_pack(&color, surf->format, surf_pack);

   uint32_t view_pack[4] = { 0, 0, 0, 0 };
   union isl_color_value swiz_color =
      isl_color_value_swizzle_inv(color, view->swizzle);
   isl_color_value_pack(&swiz_color, view->format, view_pack);

   return memcmp(surf_pack, view_pack, sizeof(surf_pack)) != 0;
}

static bool
anv_can_fast_clear_color_view(struct anv_device * device,
                              struct anv_image_view *iview,
                              VkImageLayout layout,
                              union isl_color_value clear_color,
                              uint32_t num_layers,
                              VkRect2D render_area)
{
   if (iview->planes[0].isl.base_array_layer >=
       anv_image_aux_layers(iview->image, VK_IMAGE_ASPECT_COLOR_BIT,
                            iview->planes[0].isl.base_level))
      return false;

   /* Start by getting the fast clear type.  We use the first subpass
    * layout here because we don't want to fast-clear if the first subpass
    * to use the attachment can't handle fast-clears.
    */
   enum anv_fast_clear_type fast_clear_type =
      anv_layout_to_fast_clear_type(device->info, iview->image,
                                    VK_IMAGE_ASPECT_COLOR_BIT,
                                    layout);
   switch (fast_clear_type) {
   case ANV_FAST_CLEAR_NONE:
      return false;
   case ANV_FAST_CLEAR_DEFAULT_VALUE:
      if (!isl_color_value_is_zero(clear_color, iview->planes[0].isl.format))
         return false;
      break;
   case ANV_FAST_CLEAR_ANY:
      break;
   }

   /* Potentially, we could do partial fast-clears but doing so has crazy
    * alignment restrictions.  It's easier to just restrict to full size
    * fast clears for now.
    */
   if (render_area.offset.x != 0 ||
       render_area.offset.y != 0 ||
       render_area.extent.width != iview->vk.extent.width ||
       render_area.extent.height != iview->vk.extent.height)
      return false;

   /* On Broadwell and earlier, we can only handle 0/1 clear colors */
   if (!isl_color_value_is_zero_one(clear_color, iview->planes[0].isl.format))
      return false;

   /* If the clear color is one that would require non-trivial format
    * conversion on resolve, we don't bother with the fast clear.  This
    * shouldn't be common as most clear colors are 0/1 and the most common
    * format re-interpretation is for sRGB.
    */
   if (isl_color_value_requires_conversion(clear_color,
                                           &iview->image->planes[0].primary_surface.isl,
                                           &iview->planes[0].isl)) {
      anv_perf_warn(VK_LOG_OBJS(&iview->vk.base),
                    "Cannot fast-clear to colors which would require "
                    "format conversion on resolve");
      return false;
   }

   /* We only allow fast clears to the first slice of an image (level 0,
    * layer 0) and only for the entire slice.  This guarantees us that, at
    * any given time, there is only one clear color on any given image at
    * any given time.  At the time of our testing (Jan 17, 2018), there
    * were no known applications which would benefit from fast-clearing
    * more than just the first slice.
    */
   if (iview->planes[0].isl.base_level > 0 ||
       iview->planes[0].isl.base_array_layer > 0) {
      anv_perf_warn(VK_LOG_OBJS(&iview->image->vk.base),
                    "Rendering with multi-lod or multi-layer framebuffer "
                    "with LOAD_OP_LOAD and baseMipLevel > 0 or "
                    "baseArrayLayer > 0.  Not fast clearing.");
      return false;
   }

   if (num_layers > 1) {
      anv_perf_warn(VK_LOG_OBJS(&iview->image->vk.base),
                    "Rendering to a multi-layer framebuffer with "
                    "LOAD_OP_CLEAR.  Only fast-clearing the first slice");
   }

   return true;
}

static bool
anv_can_hiz_clear_ds_view(struct anv_device *device,
                          const struct anv_image_view *iview,
                          VkImageLayout layout,
                          VkImageAspectFlags clear_aspects,
                          float depth_clear_value,
                          VkRect2D render_area)
{
   /* We don't do any HiZ or depth fast-clears on gfx7 yet */
   if (GFX_VER == 7)
      return false;

   /* If we're just clearing stencil, we can always HiZ clear */
   if (!(clear_aspects & VK_IMAGE_ASPECT_DEPTH_BIT))
      return true;

   /* We must have depth in order to have HiZ */
   if (!(iview->image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT))
      return false;

   const enum isl_aux_usage clear_aux_usage =
      anv_layout_to_aux_usage(device->info, iview->image,
                              VK_IMAGE_ASPECT_DEPTH_BIT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                              layout);
   if (!blorp_can_hiz_clear_depth(device->info,
                                  &iview->image->planes[0].primary_surface.isl,
                                  clear_aux_usage,
                                  iview->planes[0].isl.base_level,
                                  iview->planes[0].isl.base_array_layer,
                                  render_area.offset.x,
                                  render_area.offset.y,
                                  render_area.offset.x +
                                  render_area.extent.width,
                                  render_area.offset.y +
                                  render_area.extent.height))
      return false;

   if (depth_clear_value != ANV_HZ_FC_VAL)
      return false;

   /* Only gfx9+ supports returning ANV_HZ_FC_VAL when sampling a fast-cleared
    * portion of a HiZ buffer. Testing has revealed that Gfx8 only supports
    * returning 0.0f. Gens prior to gfx8 do not support this feature at all.
    */
   if (GFX_VER == 8 && anv_can_sample_with_hiz(device->info, iview->image))
      return false;

   /* If we got here, then we can fast clear */
   return true;
}

#define READ_ONCE(x) (*(volatile __typeof__(x) *)&(x))

/* Transitions a HiZ-enabled depth buffer from one layout to another. Unless
 * the initial layout is undefined, the HiZ buffer and depth buffer will
 * represent the same data at the end of this operation.
 */
static void
transition_depth_buffer(struct anv_cmd_buffer *cmd_buffer,
                        const struct anv_image *image,
                        uint32_t base_layer, uint32_t layer_count,
                        VkImageLayout initial_layout,
                        VkImageLayout final_layout,
                        bool will_full_fast_clear)
{
   const uint32_t depth_plane =
      anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_DEPTH_BIT);
   if (image->planes[depth_plane].aux_usage == ISL_AUX_USAGE_NONE)
      return;

   /* If will_full_fast_clear is set, the caller promises to fast-clear the
    * largest portion of the specified range as it can.  For depth images,
    * that means the entire image because we don't support multi-LOD HiZ.
    */
   assert(image->planes[0].primary_surface.isl.levels == 1);
   if (will_full_fast_clear)
      return;

   const enum isl_aux_state initial_state =
      anv_layout_to_aux_state(cmd_buffer->device->info, image,
                              VK_IMAGE_ASPECT_DEPTH_BIT,
                              initial_layout);
   const enum isl_aux_state final_state =
      anv_layout_to_aux_state(cmd_buffer->device->info, image,
                              VK_IMAGE_ASPECT_DEPTH_BIT,
                              final_layout);

   const bool initial_depth_valid =
      isl_aux_state_has_valid_primary(initial_state);
   const bool initial_hiz_valid =
      isl_aux_state_has_valid_aux(initial_state);
   const bool final_needs_depth =
      isl_aux_state_has_valid_primary(final_state);
   const bool final_needs_hiz =
      isl_aux_state_has_valid_aux(final_state);

   /* Getting into the pass-through state for Depth is tricky and involves
    * both a resolve and an ambiguate.  We don't handle that state right now
    * as anv_layout_to_aux_state never returns it.
    */
   assert(final_state != ISL_AUX_STATE_PASS_THROUGH);

   if (final_needs_depth && !initial_depth_valid) {
      assert(initial_hiz_valid);
      anv_image_hiz_op(cmd_buffer, image, VK_IMAGE_ASPECT_DEPTH_BIT,
                       0, base_layer, layer_count, ISL_AUX_OP_FULL_RESOLVE);
   } else if (final_needs_hiz && !initial_hiz_valid) {
      assert(initial_depth_valid);
      anv_image_hiz_op(cmd_buffer, image, VK_IMAGE_ASPECT_DEPTH_BIT,
                       0, base_layer, layer_count, ISL_AUX_OP_AMBIGUATE);
   }
}

#if GFX_VER == 7
static inline bool
vk_image_layout_stencil_write_optimal(VkImageLayout layout)
{
   return layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
          layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
          layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL ||
          layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
}
#endif

/* Transitions a HiZ-enabled depth buffer from one layout to another. Unless
 * the initial layout is undefined, the HiZ buffer and depth buffer will
 * represent the same data at the end of this operation.
 */
static void
transition_stencil_buffer(struct anv_cmd_buffer *cmd_buffer,
                          const struct anv_image *image,
                          uint32_t base_level, uint32_t level_count,
                          uint32_t base_layer, uint32_t layer_count,
                          VkImageLayout initial_layout,
                          VkImageLayout final_layout,
                          bool will_full_fast_clear)
{
#if GFX_VER == 7
   const uint32_t plane =
      anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_STENCIL_BIT);

   /* On gfx7, we have to store a texturable version of the stencil buffer in
    * a shadow whenever VK_IMAGE_USAGE_SAMPLED_BIT is set and copy back and
    * forth at strategic points. Stencil writes are only allowed in following
    * layouts:
    *
    *  - VK_IMAGE_LAYOUT_GENERAL
    *  - VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    *  - VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    *  - VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
    *  - VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
    *  - VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    *
    * For general, we have no nice opportunity to transition so we do the copy
    * to the shadow unconditionally at the end of the subpass. For transfer
    * destinations, we can update it as part of the transfer op. For the other
    * layouts, we delay the copy until a transition into some other layout.
    */
   if (anv_surface_is_valid(&image->planes[plane].shadow_surface) &&
       vk_image_layout_stencil_write_optimal(initial_layout) &&
       !vk_image_layout_stencil_write_optimal(final_layout)) {
      anv_image_copy_to_shadow(cmd_buffer, image,
                               VK_IMAGE_ASPECT_STENCIL_BIT,
                               base_level, level_count,
                               base_layer, layer_count);
   }
#endif
}

#define MI_PREDICATE_SRC0    0x2400
#define MI_PREDICATE_SRC1    0x2408
#define MI_PREDICATE_RESULT  0x2418

static void
set_image_fast_clear_state(struct anv_cmd_buffer *cmd_buffer,
                           const struct anv_image *image,
                           VkImageAspectFlagBits aspect,
                           enum anv_fast_clear_type fast_clear)
{
   anv_batch_emit(&cmd_buffer->batch, GENX(MI_STORE_DATA_IMM), sdi) {
      sdi.Address = anv_image_get_fast_clear_type_addr(cmd_buffer->device,
                                                       image, aspect);
      sdi.ImmediateData = fast_clear;
   }
}

/* This is only really practical on haswell and above because it requires
 * MI math in order to get it correct.
 */
#if GFX_VERx10 >= 75
static void
anv_cmd_compute_resolve_predicate(struct anv_cmd_buffer *cmd_buffer,
                                  const struct anv_image *image,
                                  VkImageAspectFlagBits aspect,
                                  uint32_t level, uint32_t array_layer,
                                  enum isl_aux_op resolve_op,
                                  enum anv_fast_clear_type fast_clear_supported)
{
   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   const struct mi_value fast_clear_type =
      mi_mem32(anv_image_get_fast_clear_type_addr(cmd_buffer->device,
                                                  image, aspect));

   assert(resolve_op == ISL_AUX_OP_PARTIAL_RESOLVE);
   if (level == 0 && array_layer == 0) {
      /* In this case, we are doing a partial resolve to get rid of fast-clear
       * colors.  We don't care about the compression state but we do care
       * about how much fast clear is allowed by the final layout.
       */
      assert(resolve_op == ISL_AUX_OP_PARTIAL_RESOLVE);
      assert(fast_clear_supported < ANV_FAST_CLEAR_ANY);

      /* We need to compute (fast_clear_supported < image->fast_clear) */
      struct mi_value pred =
         mi_ult(&b, mi_imm(fast_clear_supported), fast_clear_type);
      mi_store(&b, mi_reg64(MI_PREDICATE_SRC0), mi_value_ref(&b, pred));

      /* If the predicate is true, we want to write 0 to the fast clear type
       * and, if it's false, leave it alone.  We can do this by writing
       *
       * clear_type = clear_type & ~predicate;
       */
      struct mi_value new_fast_clear_type =
         mi_iand(&b, fast_clear_type, mi_inot(&b, pred));
      mi_store(&b, fast_clear_type, new_fast_clear_type);
   } else {
      /* In this case, we're trying to do a partial resolve on a slice that
       * doesn't have clear color.  There's nothing to do.
       */
      assert(resolve_op == ISL_AUX_OP_PARTIAL_RESOLVE);
      return;
   }

   /* Set src1 to 0 and use a != condition */
   mi_store(&b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(0));

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOADINV;
      mip.CombineOperation = COMBINE_SET;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }
}
#endif /* GFX_VERx10 >= 75 */

static void
anv_cmd_simple_resolve_predicate(struct anv_cmd_buffer *cmd_buffer,
                                 const struct anv_image *image,
                                 VkImageAspectFlagBits aspect,
                                 uint32_t level, uint32_t array_layer,
                                 enum isl_aux_op resolve_op,
                                 enum anv_fast_clear_type fast_clear_supported)
{
   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   struct mi_value fast_clear_type_mem =
      mi_mem32(anv_image_get_fast_clear_type_addr(cmd_buffer->device,
                                                      image, aspect));

   /* This only works for partial resolves and only when the clear color is
    * all or nothing.  On the upside, this emits less command streamer code
    * and works on Ivybridge and Bay Trail.
    */
   assert(resolve_op == ISL_AUX_OP_PARTIAL_RESOLVE);
   assert(fast_clear_supported != ANV_FAST_CLEAR_ANY);

   /* We don't support fast clears on anything other than the first slice. */
   if (level > 0 || array_layer > 0)
      return;

   /* On gfx8, we don't have a concept of default clear colors because we
    * can't sample from CCS surfaces.  It's enough to just load the fast clear
    * state into the predicate register.
    */
   mi_store(&b, mi_reg64(MI_PREDICATE_SRC0), fast_clear_type_mem);
   mi_store(&b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(0));
   mi_store(&b, fast_clear_type_mem, mi_imm(0));

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOADINV;
      mip.CombineOperation = COMBINE_SET;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }
}

static void
anv_cmd_predicated_ccs_resolve(struct anv_cmd_buffer *cmd_buffer,
                               const struct anv_image *image,
                               enum isl_format format,
                               struct isl_swizzle swizzle,
                               VkImageAspectFlagBits aspect,
                               uint32_t level, uint32_t array_layer,
                               enum isl_aux_op resolve_op,
                               enum anv_fast_clear_type fast_clear_supported)
{
   const uint32_t plane = anv_image_aspect_to_plane(image, aspect);

   anv_cmd_simple_resolve_predicate(cmd_buffer, image,
                                    aspect, level, array_layer,
                                    resolve_op, fast_clear_supported);

   /* CCS_D only supports full resolves and BLORP will assert on us if we try
    * to do a partial resolve on a CCS_D surface.
    */
   if (resolve_op == ISL_AUX_OP_PARTIAL_RESOLVE &&
       image->planes[plane].aux_usage == ISL_AUX_USAGE_CCS_D)
      resolve_op = ISL_AUX_OP_FULL_RESOLVE;

   anv_image_ccs_op(cmd_buffer, image, format, swizzle, aspect,
                    level, array_layer, 1, resolve_op, NULL, true);
}

static void
anv_cmd_predicated_mcs_resolve(struct anv_cmd_buffer *cmd_buffer,
                               const struct anv_image *image,
                               enum isl_format format,
                               struct isl_swizzle swizzle,
                               VkImageAspectFlagBits aspect,
                               uint32_t array_layer,
                               enum isl_aux_op resolve_op,
                               enum anv_fast_clear_type fast_clear_supported)
{
   assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
   assert(resolve_op == ISL_AUX_OP_PARTIAL_RESOLVE);

#if GFX_VERx10 >= 75
   anv_cmd_compute_resolve_predicate(cmd_buffer, image,
                                     aspect, 0, array_layer,
                                     resolve_op, fast_clear_supported);

   anv_image_mcs_op(cmd_buffer, image, format, swizzle, aspect,
                    array_layer, 1, resolve_op, NULL, true);
#else
   unreachable("MCS resolves are unsupported on Ivybridge and Bay Trail");
#endif
}

void
genX(cmd_buffer_mark_image_written)(struct anv_cmd_buffer *cmd_buffer,
                                    const struct anv_image *image,
                                    VkImageAspectFlagBits aspect,
                                    enum isl_aux_usage aux_usage,
                                    uint32_t level,
                                    uint32_t base_layer,
                                    uint32_t layer_count)
{
   /* The aspect must be exactly one of the image aspects. */
   assert(util_bitcount(aspect) == 1 && (aspect & image->vk.aspects));
}

static void
init_fast_clear_color(struct anv_cmd_buffer *cmd_buffer,
                      const struct anv_image *image,
                      VkImageAspectFlagBits aspect)
{
   assert(cmd_buffer && image);
   assert(image->vk.aspects & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV);

   set_image_fast_clear_state(cmd_buffer, image, aspect,
                              ANV_FAST_CLEAR_NONE);

   /* Initialize the struct fields that are accessed for fast-clears so that
    * the HW restrictions on the field values are satisfied.
    */
   struct anv_address addr =
      anv_image_get_clear_color_addr(cmd_buffer->device, image, aspect);

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_STORE_DATA_IMM), sdi) {
      sdi.Address = addr;
      if (GFX_VERx10 >= 75) {
         /* Pre-SKL, the dword containing the clear values also contains
          * other fields, so we need to initialize those fields to match the
          * values that would be in a color attachment.
          */
         sdi.ImmediateData = ISL_CHANNEL_SELECT_RED   << 25 |
                             ISL_CHANNEL_SELECT_GREEN << 22 |
                             ISL_CHANNEL_SELECT_BLUE  << 19 |
                             ISL_CHANNEL_SELECT_ALPHA << 16;
      } else if (GFX_VER == 7) {
         /* On IVB, the dword containing the clear values also contains
          * other fields that must be zero or can be zero.
          */
         sdi.ImmediateData = 0;
      }
   }
}

/* Copy the fast-clear value dword(s) between a surface state object and an
 * image's fast clear state buffer.
 */
static void
genX(copy_fast_clear_dwords)(struct anv_cmd_buffer *cmd_buffer,
                             struct anv_state surface_state,
                             const struct anv_image *image,
                             VkImageAspectFlagBits aspect,
                             bool copy_from_surface_state)
{
   assert(cmd_buffer && image);
   assert(image->vk.aspects & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV);

   struct anv_address ss_clear_addr = {
      .bo = cmd_buffer->device->surface_state_pool.block_pool.bo,
      .offset = surface_state.offset +
                cmd_buffer->device->isl_dev.ss.clear_value_offset,
   };
   const struct anv_address entry_addr =
      anv_image_get_clear_color_addr(cmd_buffer->device, image, aspect);
   unsigned copy_size = cmd_buffer->device->isl_dev.ss.clear_value_size;

#if GFX_VER == 7
   /* On gfx7, the combination of commands used here(MI_LOAD_REGISTER_MEM
    * and MI_STORE_REGISTER_MEM) can cause GPU hangs if any rendering is
    * in-flight when they are issued even if the memory touched is not
    * currently active for rendering.  The weird bit is that it is not the
    * MI_LOAD/STORE_REGISTER_MEM commands which hang but rather the in-flight
    * rendering hangs such that the next stalling command after the
    * MI_LOAD/STORE_REGISTER_MEM commands will catch the hang.
    *
    * It is unclear exactly why this hang occurs.  Both MI commands come with
    * warnings about the 3D pipeline but that doesn't seem to fully explain
    * it.  My (Faith's) best theory is that it has something to do with the
    * fact that we're using a GPU state register as our temporary and that
    * something with reading/writing it is causing problems.
    *
    * In order to work around this issue, we emit a PIPE_CONTROL with the
    * command streamer stall bit set.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_CS_STALL_BIT,
                             "after copy_fast_clear_dwords. Avoid potential hang");
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
#endif

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   if (copy_from_surface_state) {
      mi_memcpy(&b, entry_addr, ss_clear_addr, copy_size);
   } else {
      mi_memcpy(&b, ss_clear_addr, entry_addr, copy_size);

      /* Updating a surface state object may require that the state cache be
       * invalidated. From the SKL PRM, Shared Functions -> State -> State
       * Caching:
       *
       *    Whenever the RENDER_SURFACE_STATE object in memory pointed to by
       *    the Binding Table Pointer (BTP) and Binding Table Index (BTI) is
       *    modified [...], the L1 state cache must be invalidated to ensure
       *    the new surface or sampler state is fetched from system memory.
       *
       * In testing, SKL doesn't actually seem to need this, but HSW does.
       */
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_STATE_CACHE_INVALIDATE_BIT,
                                "after copy_fast_clear_dwords surface state update");
   }
}

/**
 * @brief Transitions a color buffer from one layout to another.
 *
 * See section 6.1.1. Image Layout Transitions of the Vulkan 1.0.50 spec for
 * more information.
 *
 * @param level_count VK_REMAINING_MIP_LEVELS isn't supported.
 * @param layer_count VK_REMAINING_ARRAY_LAYERS isn't supported. For 3D images,
 *                    this represents the maximum layers to transition at each
 *                    specified miplevel.
 */
static void
transition_color_buffer(struct anv_cmd_buffer *cmd_buffer,
                        const struct anv_image *image,
                        VkImageAspectFlagBits aspect,
                        const uint32_t base_level, uint32_t level_count,
                        uint32_t base_layer, uint32_t layer_count,
                        VkImageLayout initial_layout,
                        VkImageLayout final_layout,
                        uint32_t src_queue_family,
                        uint32_t dst_queue_family,
                        bool will_full_fast_clear)
{
   struct anv_device *device = cmd_buffer->device;
   const struct intel_device_info *devinfo = device->info;
   /* Validate the inputs. */
   assert(cmd_buffer);
   assert(image && image->vk.aspects & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV);
   /* These values aren't supported for simplicity's sake. */
   assert(level_count != VK_REMAINING_MIP_LEVELS &&
          layer_count != VK_REMAINING_ARRAY_LAYERS);
   /* Ensure the subresource range is valid. */
   UNUSED uint64_t last_level_num = base_level + level_count;
   const uint32_t max_depth = u_minify(image->vk.extent.depth, base_level);
   UNUSED const uint32_t image_layers = MAX2(image->vk.array_layers, max_depth);
   assert((uint64_t)base_layer + layer_count  <= image_layers);
   assert(last_level_num <= image->vk.mip_levels);
   /* If there is a layout transfer, the final layout cannot be undefined or
    * preinitialized (VUID-VkImageMemoryBarrier-newLayout-01198).
    */
   assert(initial_layout == final_layout ||
          (final_layout != VK_IMAGE_LAYOUT_UNDEFINED &&
           final_layout != VK_IMAGE_LAYOUT_PREINITIALIZED));
   const struct isl_drm_modifier_info *isl_mod_info =
      image->vk.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT
      ? isl_drm_modifier_get_info(image->vk.drm_format_mod)
      : NULL;

   const bool src_queue_external =
      src_queue_family == VK_QUEUE_FAMILY_FOREIGN_EXT ||
      src_queue_family == VK_QUEUE_FAMILY_EXTERNAL;

   const bool dst_queue_external =
      dst_queue_family == VK_QUEUE_FAMILY_FOREIGN_EXT ||
      dst_queue_family == VK_QUEUE_FAMILY_EXTERNAL;

   /* Simultaneous acquire and release on external queues is illegal. */
   assert(!src_queue_external || !dst_queue_external);

   /* Ownership transition on an external queue requires special action if the
    * image has a DRM format modifier because we store image data in
    * a driver-private bo which is inaccessible to the external queue.
    */
   const bool private_binding_acquire =
      src_queue_external &&
      anv_image_is_externally_shared(image) &&
      anv_image_has_private_binding(image);

   const bool private_binding_release =
      dst_queue_external &&
      anv_image_is_externally_shared(image) &&
      anv_image_has_private_binding(image);

   if (initial_layout == final_layout &&
       !private_binding_acquire && !private_binding_release) {
      /* No work is needed. */
       return;
   }

   const uint32_t plane = anv_image_aspect_to_plane(image, aspect);

   if (anv_surface_is_valid(&image->planes[plane].shadow_surface) &&
       final_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      /* This surface is a linear compressed image with a tiled shadow surface
       * for texturing.  The client is about to use it in READ_ONLY_OPTIMAL so
       * we need to ensure the shadow copy is up-to-date.
       */
      assert(image->vk.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT);
      assert(image->vk.aspects == VK_IMAGE_ASPECT_COLOR_BIT);
      assert(image->planes[plane].primary_surface.isl.tiling == ISL_TILING_LINEAR);
      assert(image->planes[plane].shadow_surface.isl.tiling != ISL_TILING_LINEAR);
      assert(isl_format_is_compressed(image->planes[plane].primary_surface.isl.format));
      assert(plane == 0);
      anv_image_copy_to_shadow(cmd_buffer, image,
                               VK_IMAGE_ASPECT_COLOR_BIT,
                               base_level, level_count,
                               base_layer, layer_count);
   }

   if (base_layer >= anv_image_aux_layers(image, aspect, base_level))
      return;

   assert(image->planes[plane].primary_surface.isl.tiling != ISL_TILING_LINEAR);

   /* The following layouts are equivalent for non-linear images. */
   const bool initial_layout_undefined =
      initial_layout == VK_IMAGE_LAYOUT_UNDEFINED ||
      initial_layout == VK_IMAGE_LAYOUT_PREINITIALIZED;

   bool must_init_fast_clear_state = false;
   bool must_init_aux_surface = false;

   if (initial_layout_undefined) {
      /* The subresource may have been aliased and populated with arbitrary
       * data.
       */
      must_init_fast_clear_state = true;
      must_init_aux_surface = true;
   } else if (private_binding_acquire) {
      /* The fast clear state lives in a driver-private bo, and therefore the
       * external/foreign queue is unaware of it.
       *
       * If this is the first time we are accessing the image, then the fast
       * clear state is uninitialized.
       *
       * If this is NOT the first time we are accessing the image, then the fast
       * clear state may still be valid and correct due to the resolve during
       * our most recent ownership release.  However, we do not track the aux
       * state with MI stores, and therefore must assume the worst-case: that
       * this is the first time we are accessing the image.
       */
      assert(image->planes[plane].fast_clear_memory_range.binding ==
              ANV_IMAGE_MEMORY_BINDING_PRIVATE);
      must_init_fast_clear_state = true;

      /* The aux surface, like the fast clear state, lives in
       * a driver-private bo.  We must initialize the aux surface for the
       * same reasons we must initialize the fast clear state.
       */
      assert(image->planes[plane].aux_surface.memory_range.binding ==
             ANV_IMAGE_MEMORY_BINDING_PRIVATE);
      must_init_aux_surface = true;
   }

   if (must_init_fast_clear_state) {
      if (base_level == 0 && base_layer == 0)
         init_fast_clear_color(cmd_buffer, image, aspect);
   }

   if (must_init_aux_surface) {
      assert(must_init_fast_clear_state);

      /* Initialize the aux buffers to enable correct rendering.  In order to
       * ensure that things such as storage images work correctly, aux buffers
       * need to be initialized to valid data.
       *
       * Having an aux buffer with invalid data is a problem for two reasons:
       *
       *  1) Having an invalid value in the buffer can confuse the hardware.
       *     For instance, with CCS_E on SKL, a two-bit CCS value of 2 is
       *     invalid and leads to the hardware doing strange things.  It
       *     doesn't hang as far as we can tell but rendering corruption can
       *     occur.
       *
       *  2) If this transition is into the GENERAL layout and we then use the
       *     image as a storage image, then we must have the aux buffer in the
       *     pass-through state so that, if we then go to texture from the
       *     image, we get the results of our storage image writes and not the
       *     fast clear color or other random data.
       *
       * For CCS both of the problems above are real demonstrable issues.  In
       * that case, the only thing we can do is to perform an ambiguate to
       * transition the aux surface into the pass-through state.
       *
       * For MCS, (2) is never an issue because we don't support multisampled
       * storage images.  In theory, issue (1) is a problem with MCS but we've
       * never seen it in the wild.  For 4x and 16x, all bit patters could, in
       * theory, be interpreted as something but we don't know that all bit
       * patterns are actually valid.  For 2x and 8x, you could easily end up
       * with the MCS referring to an invalid plane because not all bits of
       * the MCS value are actually used.  Even though we've never seen issues
       * in the wild, it's best to play it safe and initialize the MCS.  We
       * can use a fast-clear for MCS because we only ever touch from render
       * and texture (no image load store).
       */
      if (image->vk.samples == 1) {
         for (uint32_t l = 0; l < level_count; l++) {
            const uint32_t level = base_level + l;

            uint32_t aux_layers = anv_image_aux_layers(image, aspect, level);
            if (base_layer >= aux_layers)
               break; /* We will only get fewer layers as level increases */
            uint32_t level_layer_count =
               MIN2(layer_count, aux_layers - base_layer);

            /* If will_full_fast_clear is set, the caller promises to
             * fast-clear the largest portion of the specified range as it can.
             * For color images, that means only the first LOD and array slice.
             */
            if (level == 0 && base_layer == 0 && will_full_fast_clear) {
               base_layer++;
               level_layer_count--;
               if (level_layer_count == 0)
                  continue;
            }

            anv_image_ccs_op(cmd_buffer, image,
                             image->planes[plane].primary_surface.isl.format,
                             ISL_SWIZZLE_IDENTITY,
                             aspect, level, base_layer, level_layer_count,
                             ISL_AUX_OP_AMBIGUATE, NULL, false);
         }
      } else {
         if (image->vk.samples == 4 || image->vk.samples == 16) {
            anv_perf_warn(VK_LOG_OBJS(&image->vk.base),
                          "Doing a potentially unnecessary fast-clear to "
                          "define an MCS buffer.");
         }

         /* If will_full_fast_clear is set, the caller promises to fast-clear
          * the largest portion of the specified range as it can.
          */
         if (will_full_fast_clear)
            return;

         assert(base_level == 0 && level_count == 1);
         anv_image_mcs_op(cmd_buffer, image,
                          image->planes[plane].primary_surface.isl.format,
                          ISL_SWIZZLE_IDENTITY,
                          aspect, base_layer, layer_count,
                          ISL_AUX_OP_FAST_CLEAR, NULL, false);
      }
      return;
   }

   enum isl_aux_usage initial_aux_usage =
      anv_layout_to_aux_usage(devinfo, image, aspect, 0, initial_layout);
   enum isl_aux_usage final_aux_usage =
      anv_layout_to_aux_usage(devinfo, image, aspect, 0, final_layout);
   enum anv_fast_clear_type initial_fast_clear =
      anv_layout_to_fast_clear_type(devinfo, image, aspect, initial_layout);
   enum anv_fast_clear_type final_fast_clear =
      anv_layout_to_fast_clear_type(devinfo, image, aspect, final_layout);

   /* We must override the anv_layout_to_* functions because they are unaware of
    * acquire/release direction.
    */
   if (private_binding_acquire) {
      assert(!isl_drm_modifier_has_aux(isl_mod_info->modifier));
      initial_aux_usage = ISL_AUX_USAGE_NONE;
      initial_fast_clear = ANV_FAST_CLEAR_NONE;
   } else if (private_binding_release) {
      assert(!isl_drm_modifier_has_aux(isl_mod_info->modifier));
      final_aux_usage = ISL_AUX_USAGE_NONE;
      final_fast_clear = ANV_FAST_CLEAR_NONE;
   }

   /* The current code assumes that there is no mixing of CCS_E and CCS_D.
    * We can handle transitions between CCS_D/E to and from NONE.  What we
    * don't yet handle is switching between CCS_E and CCS_D within a given
    * image.  Doing so in a performant way requires more detailed aux state
    * tracking such as what is done in i965.  For now, just assume that we
    * only have one type of compression.
    */
   assert(initial_aux_usage == ISL_AUX_USAGE_NONE ||
          final_aux_usage == ISL_AUX_USAGE_NONE ||
          initial_aux_usage == final_aux_usage);

   /* If initial aux usage is NONE, there is nothing to resolve */
   if (initial_aux_usage == ISL_AUX_USAGE_NONE)
      return;

   enum isl_aux_op resolve_op = ISL_AUX_OP_NONE;

   /* If the initial layout supports more fast clear than the final layout
    * then we need at least a partial resolve.
    */
   if (final_fast_clear < initial_fast_clear)
      resolve_op = ISL_AUX_OP_PARTIAL_RESOLVE;

   if (resolve_op == ISL_AUX_OP_NONE)
      return;

   /* Perform a resolve to synchronize data between the main and aux buffer.
    * Before we begin, we must satisfy the cache flushing requirement specified
    * in the Sky Lake PRM Vol. 7, "MCS Buffer for Render Target(s)":
    *
    *    Any transition from any value in {Clear, Render, Resolve} to a
    *    different value in {Clear, Render, Resolve} requires end of pipe
    *    synchronization.
    *
    * We perform a flush of the write cache before and after the clear and
    * resolve operations to meet this requirement.
    *
    * Unlike other drawing, fast clear operations are not properly
    * synchronized. The first PIPE_CONTROL here likely ensures that the
    * contents of the previous render or clear hit the render target before we
    * resolve and the second likely ensures that the resolve is complete before
    * we do any more rendering or clearing.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "after transition RT");

   for (uint32_t l = 0; l < level_count; l++) {
      uint32_t level = base_level + l;

      uint32_t aux_layers = anv_image_aux_layers(image, aspect, level);
      if (base_layer >= aux_layers)
         break; /* We will only get fewer layers as level increases */
      uint32_t level_layer_count =
         MIN2(layer_count, aux_layers - base_layer);

      for (uint32_t a = 0; a < level_layer_count; a++) {
         uint32_t array_layer = base_layer + a;

         /* If will_full_fast_clear is set, the caller promises to fast-clear
          * the largest portion of the specified range as it can.  For color
          * images, that means only the first LOD and array slice.
          */
         if (level == 0 && array_layer == 0 && will_full_fast_clear)
            continue;

         if (image->vk.samples == 1) {
            anv_cmd_predicated_ccs_resolve(cmd_buffer, image,
                                           image->planes[plane].primary_surface.isl.format,
                                           ISL_SWIZZLE_IDENTITY,
                                           aspect, level, array_layer, resolve_op,
                                           final_fast_clear);
         } else {
            /* We only support fast-clear on the first layer so partial
             * resolves should not be used on other layers as they will use
             * the clear color stored in memory that is only valid for layer0.
             */
            if (resolve_op == ISL_AUX_OP_PARTIAL_RESOLVE &&
                array_layer != 0)
               continue;

            anv_cmd_predicated_mcs_resolve(cmd_buffer, image,
                                           image->planes[plane].primary_surface.isl.format,
                                           ISL_SWIZZLE_IDENTITY,
                                           aspect, array_layer, resolve_op,
                                           final_fast_clear);
         }
      }
   }

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "after transition RT");
}

static MUST_CHECK VkResult
anv_cmd_buffer_init_attachments(struct anv_cmd_buffer *cmd_buffer,
                                uint32_t color_att_count)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;

   /* Reserve one for the NULL state. */
   unsigned num_states = 1 + color_att_count;
   const struct isl_device *isl_dev = &cmd_buffer->device->isl_dev;
   const uint32_t ss_stride = align(isl_dev->ss.size, isl_dev->ss.align);
   gfx->att_states =
      anv_state_stream_alloc(&cmd_buffer->surface_state_stream,
                             num_states * ss_stride, isl_dev->ss.align);
   if (gfx->att_states.map == NULL) {
      return anv_batch_set_error(&cmd_buffer->batch,
                                 VK_ERROR_OUT_OF_DEVICE_MEMORY);
   }

   struct anv_state next_state = gfx->att_states;
   next_state.alloc_size = isl_dev->ss.size;

   gfx->null_surface_state = next_state;
   next_state.offset += ss_stride;
   next_state.map += ss_stride;

   gfx->color_att_count = color_att_count;
   for (uint32_t i = 0; i < color_att_count; i++) {
      gfx->color_att[i] = (struct anv_attachment) {
         .surface_state.state = next_state,
      };
      next_state.offset += ss_stride;
      next_state.map += ss_stride;
   }
   gfx->depth_att = (struct anv_attachment) { };
   gfx->stencil_att = (struct anv_attachment) { };

   return VK_SUCCESS;
}

static void
anv_cmd_buffer_reset_rendering(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;

   gfx->render_area = (VkRect2D) { };
   gfx->layer_count = 0;
   gfx->samples = 0;

   gfx->color_att_count = 0;
   gfx->depth_att = (struct anv_attachment) { };
   gfx->stencil_att = (struct anv_attachment) { };
   gfx->null_surface_state = ANV_STATE_NULL;
}

VkResult
genX(BeginCommandBuffer)(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   VkResult result;

   /* If this is the first vkBeginCommandBuffer, we must *initialize* the
    * command buffer's state. Otherwise, we must *reset* its state. In both
    * cases we reset it.
    *
    * From the Vulkan 1.0 spec:
    *
    *    If a command buffer is in the executable state and the command buffer
    *    was allocated from a command pool with the
    *    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag set, then
    *    vkBeginCommandBuffer implicitly resets the command buffer, behaving
    *    as if vkResetCommandBuffer had been called with
    *    VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT not set. It then puts
    *    the command buffer in the recording state.
    */
   anv_cmd_buffer_reset(&cmd_buffer->vk, 0);
   anv_cmd_buffer_reset_rendering(cmd_buffer);

   cmd_buffer->usage_flags = pBeginInfo->flags;

   /* VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT must be ignored for
    * primary level command buffers.
    *
    * From the Vulkan 1.0 spec:
    *
    *    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT specifies that a
    *    secondary command buffer is considered to be entirely inside a render
    *    pass. If this is a primary command buffer, then this bit is ignored.
    */
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      cmd_buffer->usage_flags &= ~VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

   trace_intel_begin_cmd_buffer(&cmd_buffer->trace);

   genX(cmd_buffer_emit_state_base_address)(cmd_buffer);

   /* We sometimes store vertex data in the dynamic state buffer for blorp
    * operations and our dynamic state stream may re-use data from previous
    * command buffers.  In order to prevent stale cache data, we flush the VF
    * cache.  We could do this on every blorp call but that's not really
    * needed as all of the data will get written by the CPU prior to the GPU
    * executing anything.  The chances are fairly high that they will use
    * blorp at least once per primary command buffer so it shouldn't be
    * wasted.
    *
    * There is also a workaround on gfx8 which requires us to invalidate the
    * VF cache occasionally.  It's easier if we can assume we start with a
    * fresh cache (See also genX(cmd_buffer_set_binding_for_gfx8_vb_flush).)
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_VF_CACHE_INVALIDATE_BIT,
                             "new cmd buffer");

   /* We send an "Indirect State Pointers Disable" packet at
    * EndCommandBuffer, so all push constant packets are ignored during a
    * context restore. Documentation says after that command, we need to
    * emit push constants again before any rendering operation. So we
    * flag them dirty here to make sure they get emitted.
    */
   cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_ALL_GRAPHICS;

   if (cmd_buffer->usage_flags &
       VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
      struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;

      char gcbiar_data[VK_GCBIARR_DATA_SIZE(MAX_RTS)];
      const VkRenderingInfo *resume_info =
         vk_get_command_buffer_inheritance_as_rendering_resume(cmd_buffer->vk.level,
                                                               pBeginInfo,
                                                               gcbiar_data);
      if (resume_info != NULL) {
         genX(CmdBeginRendering)(commandBuffer, resume_info);
      } else {
         const VkCommandBufferInheritanceRenderingInfo *inheritance_info =
            vk_get_command_buffer_inheritance_rendering_info(cmd_buffer->vk.level,
                                                             pBeginInfo);
         assert(inheritance_info);

         gfx->rendering_flags = inheritance_info->flags;
         gfx->render_area = (VkRect2D) { };
         gfx->layer_count = 0;
         gfx->samples = inheritance_info->rasterizationSamples;
         gfx->view_mask = inheritance_info->viewMask;

         uint32_t color_att_count = inheritance_info->colorAttachmentCount;
         result = anv_cmd_buffer_init_attachments(cmd_buffer, color_att_count);
         if (result != VK_SUCCESS)
            return result;

         for (uint32_t i = 0; i < color_att_count; i++) {
            gfx->color_att[i].vk_format =
               inheritance_info->pColorAttachmentFormats[i];
         }
         gfx->depth_att.vk_format =
            inheritance_info->depthAttachmentFormat;
         gfx->stencil_att.vk_format =
            inheritance_info->stencilAttachmentFormat;

         cmd_buffer->state.gfx.dirty |= ANV_CMD_DIRTY_RENDER_TARGETS;

         anv_cmd_graphic_state_update_has_uint_rt(gfx);
      }
   }

#if GFX_VER >= 8
   /* Emit the sample pattern at the beginning of the batch because the
    * default locations emitted at the device initialization might have been
    * changed by a previous command buffer.
    *
    * Do not change that when we're continuing a previous renderpass.
    */
   if (cmd_buffer->device->vk.enabled_extensions.EXT_sample_locations &&
       !(cmd_buffer->usage_flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT))
      genX(emit_sample_pattern)(&cmd_buffer->batch, NULL);
#endif

#if GFX_VERx10 >= 75
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
      const VkCommandBufferInheritanceConditionalRenderingInfoEXT *conditional_rendering_info =
         vk_find_struct_const(pBeginInfo->pInheritanceInfo->pNext, COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT);

      /* If secondary buffer supports conditional rendering
       * we should emit commands as if conditional rendering is enabled.
       */
      cmd_buffer->state.conditional_render_enabled =
         conditional_rendering_info && conditional_rendering_info->conditionalRenderingEnable;
   }
#endif

   return VK_SUCCESS;
}

/* From the PRM, Volume 2a:
 *
 *    "Indirect State Pointers Disable
 *
 *    At the completion of the post-sync operation associated with this pipe
 *    control packet, the indirect state pointers in the hardware are
 *    considered invalid; the indirect pointers are not saved in the context.
 *    If any new indirect state commands are executed in the command stream
 *    while the pipe control is pending, the new indirect state commands are
 *    preserved.
 *
 *    [DevIVB+]: Using Invalidate State Pointer (ISP) only inhibits context
 *    restoring of Push Constant (3DSTATE_CONSTANT_*) commands. Push Constant
 *    commands are only considered as Indirect State Pointers. Once ISP is
 *    issued in a context, SW must initialize by programming push constant
 *    commands for all the shaders (at least to zero length) before attempting
 *    any rendering operation for the same context."
 *
 * 3DSTATE_CONSTANT_* packets are restored during a context restore,
 * even though they point to a BO that has been already unreferenced at
 * the end of the previous batch buffer. This has been fine so far since
 * we are protected by these scratch page (every address not covered by
 * a BO should be pointing to the scratch page). But on CNL, it is
 * causing a GPU hang during context restore at the 3DSTATE_CONSTANT_*
 * instruction.
 *
 * The flag "Indirect State Pointers Disable" in PIPE_CONTROL tells the
 * hardware to ignore previous 3DSTATE_CONSTANT_* packets during a
 * context restore, so the mentioned hang doesn't happen. However,
 * software must program push constant commands for all stages prior to
 * rendering anything. So we flag them dirty in BeginCommandBuffer.
 *
 * Finally, we also make sure to stall at pixel scoreboard to make sure the
 * constants have been loaded into the EUs prior to disable the push constants
 * so that it doesn't hang a previous 3DPRIMITIVE.
 */
static void
emit_isp_disable(struct anv_cmd_buffer *cmd_buffer)
{
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.StallAtPixelScoreboard = true;
         pc.CommandStreamerStallEnable = true;
         anv_debug_dump_pc(pc);
   }
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.IndirectStatePointersDisable = true;
         pc.CommandStreamerStallEnable = true;
         anv_debug_dump_pc(pc);
   }
}

VkResult
genX(EndCommandBuffer)(
    VkCommandBuffer                             commandBuffer)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return cmd_buffer->batch.status;

   anv_measure_endcommandbuffer(cmd_buffer);

   /* We want every command buffer to start with the PMA fix in a known state,
    * so we disable it at the end of the command buffer.
    */
   genX(cmd_buffer_enable_pma_fix)(cmd_buffer, false);

   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   emit_isp_disable(cmd_buffer);

   trace_intel_end_cmd_buffer(&cmd_buffer->trace, cmd_buffer->vk.level);

   anv_cmd_buffer_end_batch_buffer(cmd_buffer);

   return VK_SUCCESS;
}

void
genX(CmdExecuteCommands)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCmdBuffers)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, primary, commandBuffer);

   assert(primary->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   if (anv_batch_has_error(&primary->batch))
      return;

   /* The secondary command buffers will assume that the PMA fix is disabled
    * when they begin executing.  Make sure this is true.
    */
   genX(cmd_buffer_enable_pma_fix)(primary, false);

   /* The secondary command buffer doesn't know which textures etc. have been
    * flushed prior to their execution.  Apply those flushes now.
    */
   genX(cmd_buffer_apply_pipe_flushes)(primary);

   for (uint32_t i = 0; i < commandBufferCount; i++) {
      ANV_FROM_HANDLE(anv_cmd_buffer, secondary, pCmdBuffers[i]);

      assert(secondary->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
      assert(!anv_batch_has_error(&secondary->batch));

#if GFX_VERx10 >= 75
      if (secondary->state.conditional_render_enabled) {
         if (!primary->state.conditional_render_enabled) {
            /* Secondary buffer is constructed as if it will be executed
             * with conditional rendering, we should satisfy this dependency
             * regardless of conditional rendering being enabled in primary.
             */
            struct mi_builder b;
            mi_builder_init(&b, primary->device->info, &primary->batch);
            mi_store(&b, mi_reg64(ANV_PREDICATE_RESULT_REG),
                         mi_imm(UINT64_MAX));
         }
      }
#endif

      if (secondary->usage_flags &
          VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
         /* If we're continuing a render pass from the primary, we need to
          * copy the surface states for the current subpass into the storage
          * we allocated for them in BeginCommandBuffer.
          */
         struct anv_bo *ss_bo =
            primary->device->surface_state_pool.block_pool.bo;
         struct anv_state src_state = primary->state.gfx.att_states;
         struct anv_state dst_state = secondary->state.gfx.att_states;
         assert(src_state.alloc_size == dst_state.alloc_size);

         genX(cmd_buffer_so_memcpy)(primary,
                                    (struct anv_address) {
                                       .bo = ss_bo,
                                       .offset = dst_state.offset,
                                    },
                                    (struct anv_address) {
                                       .bo = ss_bo,
                                       .offset = src_state.offset,
                                    },
                                    src_state.alloc_size);
      }

      anv_cmd_buffer_add_secondary(primary, secondary);

      assert(secondary->perf_query_pool == NULL || primary->perf_query_pool == NULL ||
             secondary->perf_query_pool == primary->perf_query_pool);
      if (secondary->perf_query_pool)
         primary->perf_query_pool = secondary->perf_query_pool;
   }

   /* The secondary isn't counted in our VF cache tracking so we need to
    * invalidate the whole thing.
    */
   if (GFX_VER == 8) {
      anv_add_pending_pipe_bits(primary,
                                ANV_PIPE_CS_STALL_BIT | ANV_PIPE_VF_CACHE_INVALIDATE_BIT,
                                "Secondary cmd buffer not tracked in VF cache");
   }

   /* The secondary may have selected a different pipeline (3D or compute) and
    * may have changed the current L3$ configuration.  Reset our tracking
    * variables to invalid values to ensure that we re-emit these in the case
    * where we do any draws or compute dispatches from the primary after the
    * secondary has returned.
    */
   primary->state.current_pipeline = UINT32_MAX;
   primary->state.current_l3_config = NULL;
   primary->state.current_hash_scale = 0;
   primary->state.gfx.push_constant_stages = 0;
   vk_dynamic_graphics_state_dirty_all(&primary->vk.dynamic_graphics_state);

   /* Each of the secondary command buffers will use its own state base
    * address.  We need to re-emit state base address for the primary after
    * all of the secondaries are done.
    *
    * TODO: Maybe we want to make this a dirty bit to avoid extra state base
    * address calls?
    */
   genX(cmd_buffer_emit_state_base_address)(primary);
}

/**
 * Program the hardware to use the specified L3 configuration.
 */
void
genX(cmd_buffer_config_l3)(struct anv_cmd_buffer *cmd_buffer,
                           const struct intel_l3_config *cfg)
{
   assert(cfg);
   if (cfg == cmd_buffer->state.current_l3_config)
      return;

   if (INTEL_DEBUG(DEBUG_L3)) {
      mesa_logd("L3 config transition: ");
      intel_dump_l3_config(cfg, stderr);
   }

   /* According to the hardware docs, the L3 partitioning can only be changed
    * while the pipeline is completely drained and the caches are flushed,
    * which involves a first PIPE_CONTROL flush which stalls the pipeline...
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.DCFlushEnable = true;
      pc.PostSyncOperation = NoWrite;
      pc.CommandStreamerStallEnable = true;
      anv_debug_dump_pc(pc);
   }

   /* ...followed by a second pipelined PIPE_CONTROL that initiates
    * invalidation of the relevant caches.  Note that because RO invalidation
    * happens at the top of the pipeline (i.e. right away as the PIPE_CONTROL
    * command is processed by the CS) we cannot combine it with the previous
    * stalling flush as the hardware documentation suggests, because that
    * would cause the CS to stall on previous rendering *after* RO
    * invalidation and wouldn't prevent the RO caches from being polluted by
    * concurrent rendering before the stall completes.  This intentionally
    * doesn't implement the SKL+ hardware workaround suggesting to enable CS
    * stall on PIPE_CONTROLs with the texture cache invalidation bit set for
    * GPGPU workloads because the previous and subsequent PIPE_CONTROLs
    * already guarantee that there is no concurrent GPGPU kernel execution
    * (see SKL HSD 2132585).
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.TextureCacheInvalidationEnable = true;
      pc.ConstantCacheInvalidationEnable = true;
      pc.InstructionCacheInvalidateEnable = true;
      pc.StateCacheInvalidationEnable = true;
      pc.PostSyncOperation = NoWrite;
      anv_debug_dump_pc(pc);
   }

   /* Now send a third stalling flush to make sure that invalidation is
    * complete when the L3 configuration registers are modified.
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.DCFlushEnable = true;
      pc.PostSyncOperation = NoWrite;
      pc.CommandStreamerStallEnable = true;
      anv_debug_dump_pc(pc);
   }

   genX(emit_l3_config)(&cmd_buffer->batch, cmd_buffer->device, cfg);
   cmd_buffer->state.current_l3_config = cfg;
}

ALWAYS_INLINE enum anv_pipe_bits
genX(emit_apply_pipe_flushes)(struct anv_batch *batch,
                              struct anv_device *device,
                              uint32_t current_pipeline,
                              enum anv_pipe_bits bits)
{
   /*
    * From Sandybridge PRM, volume 2, "1.7.2 End-of-Pipe Synchronization":
    *
    *    Write synchronization is a special case of end-of-pipe
    *    synchronization that requires that the render cache and/or depth
    *    related caches are flushed to memory, where the data will become
    *    globally visible. This type of synchronization is required prior to
    *    SW (CPU) actually reading the result data from memory, or initiating
    *    an operation that will use as a read surface (such as a texture
    *    surface) a previous render target and/or depth/stencil buffer
    *
    *
    * From Haswell PRM, volume 2, part 1, "End-of-Pipe Synchronization":
    *
    *    Exercising the write cache flush bits (Render Target Cache Flush
    *    Enable, Depth Cache Flush Enable, DC Flush) in PIPE_CONTROL only
    *    ensures the write caches are flushed and doesn't guarantee the data
    *    is globally visible.
    *
    *    SW can track the completion of the end-of-pipe-synchronization by
    *    using "Notify Enable" and "PostSync Operation - Write Immediate
    *    Data" in the PIPE_CONTROL command.
    *
    * In other words, flushes are pipelined while invalidations are handled
    * immediately.  Therefore, if we're flushing anything then we need to
    * schedule an end-of-pipe sync before any invalidations can happen.
    */
   if (bits & ANV_PIPE_FLUSH_BITS)
      bits |= ANV_PIPE_NEEDS_END_OF_PIPE_SYNC_BIT;

   /* If we're going to do an invalidate and we have a pending end-of-pipe
    * sync that has yet to be resolved, we do the end-of-pipe sync now.
    */
   if ((bits & ANV_PIPE_INVALIDATE_BITS) &&
       (bits & ANV_PIPE_NEEDS_END_OF_PIPE_SYNC_BIT)) {
      bits |= ANV_PIPE_END_OF_PIPE_SYNC_BIT;
      bits &= ~ANV_PIPE_NEEDS_END_OF_PIPE_SYNC_BIT;
   }

   /* Project: SKL / Argument: LRI Post Sync Operation [23]
    *
    * "PIPECONTROL command with “Command Streamer Stall Enable” must be
    *  programmed prior to programming a PIPECONTROL command with "LRI
    *  Post Sync Operation" in GPGPU mode of operation (i.e when
    *  PIPELINE_SELECT command is set to GPGPU mode of operation)."
    *
    * The same text exists a few rows below for Post Sync Op.
    */
   if (bits & ANV_PIPE_POST_SYNC_BIT)
      bits &= ~ANV_PIPE_POST_SYNC_BIT;

   if (bits & (ANV_PIPE_FLUSH_BITS | ANV_PIPE_STALL_BITS |
               ANV_PIPE_END_OF_PIPE_SYNC_BIT)) {
      anv_batch_emit(batch, GENX(PIPE_CONTROL), pipe) {
         /* Flushing HDC pipeline requires DC Flush on earlier HW. */
         pipe.DCFlushEnable |= bits & ANV_PIPE_HDC_PIPELINE_FLUSH_BIT;
         pipe.DepthCacheFlushEnable = bits & ANV_PIPE_DEPTH_CACHE_FLUSH_BIT;
         pipe.DCFlushEnable |= bits & ANV_PIPE_DATA_CACHE_FLUSH_BIT;
         pipe.RenderTargetCacheFlushEnable =
            bits & ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT;

         pipe.CommandStreamerStallEnable = bits & ANV_PIPE_CS_STALL_BIT;
#if GFX_VER == 8
         /* From Broadwell PRM, volume 2a:
          *    PIPE_CONTROL: Command Streamer Stall Enable:
          *
          *    "This bit must be always set when PIPE_CONTROL command is
          *     programmed by GPGPU and MEDIA workloads, except for the cases
          *     when only Read Only Cache Invalidation bits are set (State
          *     Cache Invalidation Enable, Instruction cache Invalidation
          *     Enable, Texture Cache Invalidation Enable, Constant Cache
          *     Invalidation Enable). This is to WA FFDOP CG issue, this WA
          *     need not implemented when FF_DOP_CG is disabled."
          *
          *    Since we do all the invalidation in the following PIPE_CONTROL,
          *    if we got here, we need a stall.
          */
         pipe.CommandStreamerStallEnable |= current_pipeline == GPGPU;
#endif

         pipe.StallAtPixelScoreboard = bits & ANV_PIPE_STALL_AT_SCOREBOARD_BIT;

         /* From Sandybridge PRM, volume 2, "1.7.3.1 Writing a Value to Memory":
          *
          *    "The most common action to perform upon reaching a
          *    synchronization point is to write a value out to memory. An
          *    immediate value (included with the synchronization command) may
          *    be written."
          *
          *
          * From Broadwell PRM, volume 7, "End-of-Pipe Synchronization":
          *
          *    "In case the data flushed out by the render engine is to be
          *    read back in to the render engine in coherent manner, then the
          *    render engine has to wait for the fence completion before
          *    accessing the flushed data. This can be achieved by following
          *    means on various products: PIPE_CONTROL command with CS Stall
          *    and the required write caches flushed with Post-Sync-Operation
          *    as Write Immediate Data.
          *
          *    Example:
          *       - Workload-1 (3D/GPGPU/MEDIA)
          *       - PIPE_CONTROL (CS Stall, Post-Sync-Operation Write
          *         Immediate Data, Required Write Cache Flush bits set)
          *       - Workload-2 (Can use the data produce or output by
          *         Workload-1)
          */
         if (bits & ANV_PIPE_END_OF_PIPE_SYNC_BIT) {
            pipe.CommandStreamerStallEnable = true;
            pipe.PostSyncOperation = WriteImmediateData;
            pipe.Address = device->workaround_address;
         }

         /*
          * According to the Broadwell documentation, any PIPE_CONTROL with the
          * "Command Streamer Stall" bit set must also have another bit set,
          * with five different options:
          *
          *  - Render Target Cache Flush
          *  - Depth Cache Flush
          *  - Stall at Pixel Scoreboard
          *  - Post-Sync Operation
          *  - Depth Stall
          *  - DC Flush Enable
          *
          * I chose "Stall at Pixel Scoreboard" since that's what we use in
          * mesa and it seems to work fine. The choice is fairly arbitrary.
          */
         if (pipe.CommandStreamerStallEnable &&
             !pipe.RenderTargetCacheFlushEnable &&
             !pipe.DepthCacheFlushEnable &&
             !pipe.StallAtPixelScoreboard &&
             !pipe.PostSyncOperation &&
             !pipe.DepthStallEnable &&
             !pipe.DCFlushEnable)
            pipe.StallAtPixelScoreboard = true;
         anv_debug_dump_pc(pipe);
      }

      /* If a render target flush was emitted, then we can toggle off the bit
       * saying that render target writes are ongoing.
       */
      if (bits & ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT)
         bits &= ~(ANV_PIPE_RENDER_TARGET_BUFFER_WRITES);

      if (GFX_VERx10 == 75) {
         /* Haswell needs addition work-arounds:
          *
          * From Haswell PRM, volume 2, part 1, "End-of-Pipe Synchronization":
          *
          *    Option 1:
          *    PIPE_CONTROL command with the CS Stall and the required write
          *    caches flushed with Post-SyncOperation as Write Immediate Data
          *    followed by eight dummy MI_STORE_DATA_IMM (write to scratch
          *    spce) commands.
          *
          *    Example:
          *       - Workload-1
          *       - PIPE_CONTROL (CS Stall, Post-Sync-Operation Write
          *         Immediate Data, Required Write Cache Flush bits set)
          *       - MI_STORE_DATA_IMM (8 times) (Dummy data, Scratch Address)
          *       - Workload-2 (Can use the data produce or output by
          *         Workload-1)
          *
          * Unfortunately, both the PRMs and the internal docs are a bit
          * out-of-date in this regard.  What the windows driver does (and
          * this appears to actually work) is to emit a register read from the
          * memory address written by the pipe control above.
          *
          * What register we load into doesn't matter.  We choose an indirect
          * rendering register because we know it always exists and it's one
          * of the first registers the command parser allows us to write.  If
          * you don't have command parser support in your kernel (pre-4.2),
          * this will get turned into MI_NOOP and you won't get the
          * workaround.  Unfortunately, there's just not much we can do in
          * that case.  This register is perfectly safe to write since we
          * always re-load all of the indirect draw registers right before
          * 3DPRIMITIVE when needed anyway.
          */
         anv_batch_emit(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
            lrm.RegisterAddress  = 0x243C; /* GFX7_3DPRIM_START_INSTANCE */
            lrm.MemoryAddress = device->workaround_address;
         }
      }

      bits &= ~(ANV_PIPE_FLUSH_BITS | ANV_PIPE_STALL_BITS |
                ANV_PIPE_END_OF_PIPE_SYNC_BIT);
   }

   if (bits & ANV_PIPE_INVALIDATE_BITS) {
      anv_batch_emit(batch, GENX(PIPE_CONTROL), pipe) {
         pipe.StateCacheInvalidationEnable =
            bits & ANV_PIPE_STATE_CACHE_INVALIDATE_BIT;
         pipe.ConstantCacheInvalidationEnable =
            bits & ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT;
         pipe.VFCacheInvalidationEnable =
            bits & ANV_PIPE_VF_CACHE_INVALIDATE_BIT;
         pipe.TextureCacheInvalidationEnable =
            bits & ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT;
         pipe.InstructionCacheInvalidateEnable =
            bits & ANV_PIPE_INSTRUCTION_CACHE_INVALIDATE_BIT;

         anv_debug_dump_pc(pipe);
      }

      bits &= ~ANV_PIPE_INVALIDATE_BITS;
   }

   return bits;
}

ALWAYS_INLINE void
genX(cmd_buffer_apply_pipe_flushes)(struct anv_cmd_buffer *cmd_buffer)
{
   enum anv_pipe_bits bits = cmd_buffer->state.pending_pipe_bits;

   if (unlikely(cmd_buffer->device->physical->always_flush_cache))
      bits |= ANV_PIPE_FLUSH_BITS | ANV_PIPE_INVALIDATE_BITS;
   else if (bits == 0)
      return;

   bool trace_flush =
      (bits & (ANV_PIPE_FLUSH_BITS | ANV_PIPE_STALL_BITS | ANV_PIPE_INVALIDATE_BITS)) != 0;
   if (trace_flush)
      trace_intel_begin_stall(&cmd_buffer->trace);

   if (GFX_VER == 8 &&
       (bits & ANV_PIPE_CS_STALL_BIT) &&
       (bits & ANV_PIPE_VF_CACHE_INVALIDATE_BIT)) {
      /* If we are doing a VF cache invalidate AND a CS stall (it must be
       * both) then we can reset our vertex cache tracking.
       */
      memset(cmd_buffer->state.gfx.vb_dirty_ranges, 0,
             sizeof(cmd_buffer->state.gfx.vb_dirty_ranges));
      memset(&cmd_buffer->state.gfx.ib_dirty_range, 0,
             sizeof(cmd_buffer->state.gfx.ib_dirty_range));
   }

   cmd_buffer->state.pending_pipe_bits =
      genX(emit_apply_pipe_flushes)(&cmd_buffer->batch,
                                    cmd_buffer->device,
                                    cmd_buffer->state.current_pipeline,
                                    bits);

   if (trace_flush) {
      trace_intel_end_stall(&cmd_buffer->trace, bits,
                            anv_pipe_flush_bit_to_ds_stall_flag, NULL);
   }
}

static void
cmd_buffer_barrier(struct anv_cmd_buffer *cmd_buffer,
                   const VkDependencyInfo *dep_info,
                   const char *reason)
{
   /* XXX: Right now, we're really dumb and just flush whatever categories
    * the app asks for.  One of these days we may make this a bit better
    * but right now that's all the hardware allows for in most areas.
    */
   VkAccessFlags2 src_flags = 0;
   VkAccessFlags2 dst_flags = 0;

   for (uint32_t i = 0; i < dep_info->memoryBarrierCount; i++) {
      src_flags |= dep_info->pMemoryBarriers[i].srcAccessMask;
      dst_flags |= dep_info->pMemoryBarriers[i].dstAccessMask;
   }

   for (uint32_t i = 0; i < dep_info->bufferMemoryBarrierCount; i++) {
      src_flags |= dep_info->pBufferMemoryBarriers[i].srcAccessMask;
      dst_flags |= dep_info->pBufferMemoryBarriers[i].dstAccessMask;
   }

   for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; i++) {
      const VkImageMemoryBarrier2 *img_barrier =
         &dep_info->pImageMemoryBarriers[i];

      src_flags |= img_barrier->srcAccessMask;
      dst_flags |= img_barrier->dstAccessMask;

      ANV_FROM_HANDLE(anv_image, image, img_barrier->image);
      const VkImageSubresourceRange *range = &img_barrier->subresourceRange;

      uint32_t base_layer, layer_count;
      if (image->vk.image_type == VK_IMAGE_TYPE_3D) {
         base_layer = 0;
         layer_count = u_minify(image->vk.extent.depth, range->baseMipLevel);
      } else {
         base_layer = range->baseArrayLayer;
         layer_count = vk_image_subresource_layer_count(&image->vk, range);
      }
      const uint32_t level_count =
         vk_image_subresource_level_count(&image->vk, range);

      if (range->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         transition_depth_buffer(cmd_buffer, image,
                                 base_layer, layer_count,
                                 img_barrier->oldLayout,
                                 img_barrier->newLayout,
                                 false /* will_full_fast_clear */);
      }

      if (range->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
         transition_stencil_buffer(cmd_buffer, image,
                                   range->baseMipLevel, level_count,
                                   base_layer, layer_count,
                                   img_barrier->oldLayout,
                                   img_barrier->newLayout,
                                   false /* will_full_fast_clear */);

         /* If we are in a renderpass, the gfx7 stencil shadow may need to be
          * updated even if the layout doesn't change
          */
         if (cmd_buffer->state.gfx.samples &&
              (img_barrier->dstAccessMask & (VK_ACCESS_2_SHADER_READ_BIT |
                                             VK_ACCESS_2_SHADER_SAMPLED_READ_BIT |
                                             VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT))) {
            const uint32_t plane =
               anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_STENCIL_BIT);
            if (anv_surface_is_valid(&image->planes[plane].shadow_surface))
               anv_image_copy_to_shadow(cmd_buffer, image,
                                        VK_IMAGE_ASPECT_STENCIL_BIT,
                                        range->baseMipLevel, level_count,
                                        base_layer, layer_count);
         }
      }

      if (range->aspectMask & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV) {
         VkImageAspectFlags color_aspects =
            vk_image_expand_aspect_mask(&image->vk, range->aspectMask);
         anv_foreach_image_aspect_bit(aspect_bit, image, color_aspects) {
            transition_color_buffer(cmd_buffer, image, 1UL << aspect_bit,
                                    range->baseMipLevel, level_count,
                                    base_layer, layer_count,
                                    img_barrier->oldLayout,
                                    img_barrier->newLayout,
                                    img_barrier->srcQueueFamilyIndex,
                                    img_barrier->dstQueueFamilyIndex,
                                    false /* will_full_fast_clear */);
         }
      }
   }

   enum anv_pipe_bits bits =
      anv_pipe_flush_bits_for_access_flags(cmd_buffer->device, src_flags) |
      anv_pipe_invalidate_bits_for_access_flags(cmd_buffer->device, dst_flags);

   anv_add_pending_pipe_bits(cmd_buffer, bits, reason);
}

void genX(CmdPipelineBarrier2)(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   cmd_buffer_barrier(cmd_buffer, pDependencyInfo, "pipe barrier");
}

static void
cmd_buffer_alloc_push_constants(struct anv_cmd_buffer *cmd_buffer)
{
   VkShaderStageFlags stages =
      cmd_buffer->state.gfx.pipeline->active_stages;

   /* In order to avoid thrash, we assume that vertex and fragment stages
    * always exist.  In the rare case where one is missing *and* the other
    * uses push concstants, this may be suboptimal.  However, avoiding stalls
    * seems more important.
    */
   stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
   if (anv_pipeline_is_primitive(cmd_buffer->state.gfx.pipeline))
      stages |= VK_SHADER_STAGE_VERTEX_BIT;

   if (stages == cmd_buffer->state.gfx.push_constant_stages)
      return;

   const unsigned push_constant_kb =
      cmd_buffer->device->info->max_constant_urb_size_kb;

   const unsigned num_stages =
      util_bitcount(stages & VK_SHADER_STAGE_ALL_GRAPHICS);
   unsigned size_per_stage = push_constant_kb / num_stages;

   /* Broadwell+ and Haswell gt3 require that the push constant sizes be in
    * units of 2KB.  Incidentally, these are the same platforms that have
    * 32KB worth of push constant space.
    */
   if (push_constant_kb == 32)
      size_per_stage &= ~1u;

   uint32_t kb_used = 0;
   for (int i = MESA_SHADER_VERTEX; i < MESA_SHADER_FRAGMENT; i++) {
      unsigned push_size = (stages & (1 << i)) ? size_per_stage : 0;
      anv_batch_emit(&cmd_buffer->batch,
                     GENX(3DSTATE_PUSH_CONSTANT_ALLOC_VS), alloc) {
         alloc._3DCommandSubOpcode  = 18 + i;
         alloc.ConstantBufferOffset = (push_size > 0) ? kb_used : 0;
         alloc.ConstantBufferSize   = push_size;
      }
      kb_used += push_size;
   }

   anv_batch_emit(&cmd_buffer->batch,
                  GENX(3DSTATE_PUSH_CONSTANT_ALLOC_PS), alloc) {
      alloc.ConstantBufferOffset = kb_used;
      alloc.ConstantBufferSize = push_constant_kb - kb_used;
   }

   cmd_buffer->state.gfx.push_constant_stages = stages;

   /* From the BDW PRM for 3DSTATE_PUSH_CONSTANT_ALLOC_VS:
    *
    *    "The 3DSTATE_CONSTANT_VS must be reprogrammed prior to
    *    the next 3DPRIMITIVE command after programming the
    *    3DSTATE_PUSH_CONSTANT_ALLOC_VS"
    *
    * Since 3DSTATE_PUSH_CONSTANT_ALLOC_VS is programmed as part of
    * pipeline setup, we need to dirty push constants.
    */
   cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_ALL_GRAPHICS;
}

static VkResult
emit_binding_table(struct anv_cmd_buffer *cmd_buffer,
                   struct anv_cmd_pipeline_state *pipe_state,
                   struct anv_shader_bin *shader,
                   struct anv_state *bt_state)
{
   uint32_t state_offset;

   struct anv_pipeline_bind_map *map = &shader->bind_map;
   if (map->surface_count == 0) {
      *bt_state = (struct anv_state) { 0, };
      return VK_SUCCESS;
   }

   *bt_state = anv_cmd_buffer_alloc_binding_table(cmd_buffer,
                                                  map->surface_count,
                                                  &state_offset);
   uint32_t *bt_map = bt_state->map;

   if (bt_state->map == NULL)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   /* We only need to emit relocs if we're not using softpin.  If we are using
    * softpin then we always keep all user-allocated memory objects resident.
    */
   const bool need_client_mem_relocs =
      anv_use_relocations(cmd_buffer->device->physical);
   struct anv_push_constants *push = &pipe_state->push_constants;

   for (uint32_t s = 0; s < map->surface_count; s++) {
      struct anv_pipeline_binding *binding = &map->surface_to_descriptor[s];

      struct anv_state surface_state;

      switch (binding->set) {
      case ANV_DESCRIPTOR_SET_NULL:
         bt_map[s] = 0;
         break;

      case ANV_DESCRIPTOR_SET_COLOR_ATTACHMENTS:
         /* Color attachment binding */
         assert(shader->stage == MESA_SHADER_FRAGMENT);
         if (binding->index < cmd_buffer->state.gfx.color_att_count) {
            const struct anv_attachment *att =
               &cmd_buffer->state.gfx.color_att[binding->index];
            surface_state = att->surface_state.state;
         } else {
            surface_state = cmd_buffer->state.gfx.null_surface_state;
         }
         assert(surface_state.map);
         bt_map[s] = surface_state.offset + state_offset;
         break;

      case ANV_DESCRIPTOR_SET_SHADER_CONSTANTS: {
         struct anv_state surface_state =
            anv_cmd_buffer_alloc_surface_state(cmd_buffer);

         struct anv_address constant_data = {
            .bo = cmd_buffer->device->instruction_state_pool.block_pool.bo,
            .offset = shader->kernel.offset +
                      shader->prog_data->const_data_offset,
         };
         unsigned constant_data_size = shader->prog_data->const_data_size;

         const enum isl_format format =
            anv_isl_format_for_descriptor_type(cmd_buffer->device,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
         anv_fill_buffer_surface_state(cmd_buffer->device, surface_state,
                                       format, ISL_SWIZZLE_IDENTITY,
                                       ISL_SURF_USAGE_CONSTANT_BUFFER_BIT,
                                       constant_data, constant_data_size, 1);

         assert(surface_state.map);
         bt_map[s] = surface_state.offset + state_offset;
         add_surface_reloc(cmd_buffer, surface_state, constant_data);
         break;
      }

      case ANV_DESCRIPTOR_SET_NUM_WORK_GROUPS: {
         /* This is always the first binding for compute shaders */
         assert(shader->stage == MESA_SHADER_COMPUTE && s == 0);

         struct anv_state surface_state =
            anv_cmd_buffer_alloc_surface_state(cmd_buffer);

         const enum isl_format format =
            anv_isl_format_for_descriptor_type(cmd_buffer->device,
                                               VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
         anv_fill_buffer_surface_state(cmd_buffer->device, surface_state,
                                       format, ISL_SWIZZLE_IDENTITY,
                                       ISL_SURF_USAGE_CONSTANT_BUFFER_BIT,
                                       cmd_buffer->state.compute.num_workgroups,
                                       12, 1);

         assert(surface_state.map);
         bt_map[s] = surface_state.offset + state_offset;
         if (need_client_mem_relocs) {
            add_surface_reloc(cmd_buffer, surface_state,
                              cmd_buffer->state.compute.num_workgroups);
         }
         break;
      }

      case ANV_DESCRIPTOR_SET_DESCRIPTORS: {
         /* This is a descriptor set buffer so the set index is actually
          * given by binding->binding.  (Yes, that's confusing.)
          */
         struct anv_descriptor_set *set =
            pipe_state->descriptors[binding->index];
         assert(set->desc_mem.alloc_size);
         assert(set->desc_surface_state.alloc_size);
         bt_map[s] = set->desc_surface_state.offset + state_offset;
         add_surface_reloc(cmd_buffer, set->desc_surface_state,
                           anv_descriptor_set_address(set));
         break;
      }

      default: {
         assert(binding->set < MAX_SETS);
         const struct anv_descriptor_set *set =
            pipe_state->descriptors[binding->set];
         if (binding->index >= set->descriptor_count) {
            /* From the Vulkan spec section entitled "DescriptorSet and
             * Binding Assignment":
             *
             *    "If the array is runtime-sized, then array elements greater
             *    than or equal to the size of that binding in the bound
             *    descriptor set must not be used."
             *
             * Unfortunately, the compiler isn't smart enough to figure out
             * when a dynamic binding isn't used so it may grab the whole
             * array and stick it in the binding table.  In this case, it's
             * safe to just skip those bindings that are OOB.
             */
            assert(binding->index < set->layout->descriptor_count);
            continue;
         }
         const struct anv_descriptor *desc = &set->descriptors[binding->index];

         switch (desc->type) {
         case VK_DESCRIPTOR_TYPE_SAMPLER:
            /* Nothing for us to do here */
            continue;

         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            if (desc->image_view) {
               struct anv_surface_state sstate =
                  (desc->layout == VK_IMAGE_LAYOUT_GENERAL) ?
                  desc->image_view->planes[binding->plane].general_sampler_surface_state :
                  desc->image_view->planes[binding->plane].optimal_sampler_surface_state;
               surface_state = sstate.state;
               assert(surface_state.alloc_size);
               if (need_client_mem_relocs)
                  add_surface_state_relocs(cmd_buffer, sstate);
            } else {
               surface_state = cmd_buffer->device->null_surface_state;
            }
            break;
         }

         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            if (desc->image_view) {
               struct anv_surface_state sstate =
                  binding->lowered_storage_surface
                  ? desc->image_view->planes[binding->plane].lowered_storage_surface_state
                  : desc->image_view->planes[binding->plane].storage_surface_state;
               surface_state = sstate.state;
               assert(surface_state.alloc_size);
               if (surface_state.offset == 0) {
                  mesa_loge("Bound a image to a descriptor where the "
                            "descriptor does not have NonReadable "
                            "set and the image does not have a "
                            "corresponding SPIR-V format enum.");
                  vk_debug_report(&cmd_buffer->device->physical->instance->vk,
                                  VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                  &desc->image_view->vk.base,
                                  __LINE__, 0, "anv",
                                  "Bound a image to a descriptor where the "
                                  "descriptor does not have NonReadable "
                                  "set and the image does not have a "
                                  "corresponding SPIR-V format enum.");
               }
               if (surface_state.offset && need_client_mem_relocs)
                  add_surface_state_relocs(cmd_buffer, sstate);
            } else {
               surface_state = cmd_buffer->device->null_surface_state;
            }
            break;
         }

         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            if (desc->set_buffer_view) {
               surface_state = desc->set_buffer_view->surface_state;
               assert(surface_state.alloc_size);
               if (need_client_mem_relocs) {
                  add_surface_reloc(cmd_buffer, surface_state,
                                    desc->set_buffer_view->address);
               }
            } else {
               surface_state = cmd_buffer->device->null_surface_state;
            }
            break;

         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (desc->buffer_view) {
               surface_state = desc->buffer_view->surface_state;
               assert(surface_state.alloc_size);
               if (need_client_mem_relocs) {
                  add_surface_reloc(cmd_buffer, surface_state,
                                    desc->buffer_view->address);
               }
            } else {
               surface_state = cmd_buffer->device->null_surface_state;
            }
            break;

         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            if (desc->buffer) {
               /* Compute the offset within the buffer */
               uint32_t dynamic_offset =
                  push->dynamic_offsets[binding->dynamic_offset_index];
               uint64_t offset = desc->offset + dynamic_offset;
               /* Clamp to the buffer size */
               offset = MIN2(offset, desc->buffer->vk.size);
               /* Clamp the range to the buffer size */
               uint32_t range = MIN2(desc->range, desc->buffer->vk.size - offset);

               /* Align the range for consistency */
               if (desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
                  range = align(range, ANV_UBO_ALIGNMENT);

               struct anv_address address =
                  anv_address_add(desc->buffer->address, offset);

               surface_state =
                  anv_state_stream_alloc(&cmd_buffer->surface_state_stream, 64, 64);
               enum isl_format format =
                  anv_isl_format_for_descriptor_type(cmd_buffer->device,
                                                     desc->type);

               isl_surf_usage_flags_t usage =
                  desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ?
                  ISL_SURF_USAGE_CONSTANT_BUFFER_BIT :
                  ISL_SURF_USAGE_STORAGE_BIT;

               anv_fill_buffer_surface_state(cmd_buffer->device, surface_state,
                                             format, ISL_SWIZZLE_IDENTITY,
                                             usage, address, range, 1);
               if (need_client_mem_relocs)
                  add_surface_reloc(cmd_buffer, surface_state, address);
            } else {
               surface_state = cmd_buffer->device->null_surface_state;
            }
            break;
         }

         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (desc->buffer_view) {
               surface_state = binding->lowered_storage_surface
                  ? desc->buffer_view->lowered_storage_surface_state
                  : desc->buffer_view->storage_surface_state;
               assert(surface_state.alloc_size);
               if (need_client_mem_relocs) {
                  add_surface_reloc(cmd_buffer, surface_state,
                                    desc->buffer_view->address);
               }
            } else {
               surface_state = cmd_buffer->device->null_surface_state;
            }
            break;

         default:
            assert(!"Invalid descriptor type");
            continue;
         }
         assert(surface_state.map);
         bt_map[s] = surface_state.offset + state_offset;
         break;
      }
      }
   }

   return VK_SUCCESS;
}

static VkResult
emit_samplers(struct anv_cmd_buffer *cmd_buffer,
              struct anv_cmd_pipeline_state *pipe_state,
              struct anv_shader_bin *shader,
              struct anv_state *state)
{
   struct anv_pipeline_bind_map *map = &shader->bind_map;
   if (map->sampler_count == 0) {
      *state = (struct anv_state) { 0, };
      return VK_SUCCESS;
   }

   uint32_t size = map->sampler_count * 16;
   *state = anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, size, 32);

   if (state->map == NULL)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   for (uint32_t s = 0; s < map->sampler_count; s++) {
      struct anv_pipeline_binding *binding = &map->sampler_to_descriptor[s];
      const struct anv_descriptor *desc =
         &pipe_state->descriptors[binding->set]->descriptors[binding->index];

      if (desc->type != VK_DESCRIPTOR_TYPE_SAMPLER &&
          desc->type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
         continue;

      struct anv_sampler *sampler = desc->sampler;

      /* This can happen if we have an unfilled slot since TYPE_SAMPLER
       * happens to be zero.
       */
      if (sampler == NULL)
         continue;

      memcpy(state->map + (s * 16),
             sampler->state[binding->plane], sizeof(sampler->state[0]));
   }

   return VK_SUCCESS;
}

static uint32_t
flush_descriptor_sets(struct anv_cmd_buffer *cmd_buffer,
                      struct anv_cmd_pipeline_state *pipe_state,
                      const VkShaderStageFlags dirty,
                      struct anv_shader_bin **shaders,
                      uint32_t num_shaders)
{
   VkShaderStageFlags flushed = 0;

   VkResult result = VK_SUCCESS;
   for (uint32_t i = 0; i < num_shaders; i++) {
      if (!shaders[i])
         continue;

      gl_shader_stage stage = shaders[i]->stage;
      VkShaderStageFlags vk_stage = mesa_to_vk_shader_stage(stage);
      if ((vk_stage & dirty) == 0)
         continue;

      assert(stage < ARRAY_SIZE(cmd_buffer->state.samplers));
      result = emit_samplers(cmd_buffer, pipe_state, shaders[i],
                             &cmd_buffer->state.samplers[stage]);
      if (result != VK_SUCCESS)
         break;

      assert(stage < ARRAY_SIZE(cmd_buffer->state.binding_tables));
      result = emit_binding_table(cmd_buffer, pipe_state, shaders[i],
                                  &cmd_buffer->state.binding_tables[stage]);
      if (result != VK_SUCCESS)
         break;

      flushed |= vk_stage;
   }

   if (result != VK_SUCCESS) {
      assert(result == VK_ERROR_OUT_OF_DEVICE_MEMORY);

      result = anv_cmd_buffer_new_binding_table_block(cmd_buffer);
      if (result != VK_SUCCESS)
         return 0;

      /* Re-emit state base addresses so we get the new surface state base
       * address before we start emitting binding tables etc.
       */
      genX(cmd_buffer_emit_state_base_address)(cmd_buffer);

      /* Re-emit all active binding tables */
      flushed = 0;

      for (uint32_t i = 0; i < num_shaders; i++) {
         if (!shaders[i])
            continue;

         gl_shader_stage stage = shaders[i]->stage;

         result = emit_samplers(cmd_buffer, pipe_state, shaders[i],
                                &cmd_buffer->state.samplers[stage]);
         if (result != VK_SUCCESS) {
            anv_batch_set_error(&cmd_buffer->batch, result);
            return 0;
         }
         result = emit_binding_table(cmd_buffer, pipe_state, shaders[i],
                                     &cmd_buffer->state.binding_tables[stage]);
         if (result != VK_SUCCESS) {
            anv_batch_set_error(&cmd_buffer->batch, result);
            return 0;
         }

         flushed |= mesa_to_vk_shader_stage(stage);
      }
   }

   return flushed;
}

static void
cmd_buffer_emit_descriptor_pointers(struct anv_cmd_buffer *cmd_buffer,
                                    uint32_t stages)
{
   static const uint32_t sampler_state_opcodes[] = {
      [MESA_SHADER_VERTEX]                      = 43,
      [MESA_SHADER_TESS_CTRL]                   = 44, /* HS */
      [MESA_SHADER_TESS_EVAL]                   = 45, /* DS */
      [MESA_SHADER_GEOMETRY]                    = 46,
      [MESA_SHADER_FRAGMENT]                    = 47,
   };

   static const uint32_t binding_table_opcodes[] = {
      [MESA_SHADER_VERTEX]                      = 38,
      [MESA_SHADER_TESS_CTRL]                   = 39,
      [MESA_SHADER_TESS_EVAL]                   = 40,
      [MESA_SHADER_GEOMETRY]                    = 41,
      [MESA_SHADER_FRAGMENT]                    = 42,
   };

   anv_foreach_stage(s, stages) {
      assert(s < ARRAY_SIZE(binding_table_opcodes));

      if (cmd_buffer->state.samplers[s].alloc_size > 0) {
         anv_batch_emit(&cmd_buffer->batch,
                        GENX(3DSTATE_SAMPLER_STATE_POINTERS_VS), ssp) {
            ssp._3DCommandSubOpcode = sampler_state_opcodes[s];
            ssp.PointertoVSSamplerState = cmd_buffer->state.samplers[s].offset;
         }
      }

      /* Always emit binding table pointers if we're asked to, since on SKL
       * this is what flushes push constants. */
      anv_batch_emit(&cmd_buffer->batch,
                     GENX(3DSTATE_BINDING_TABLE_POINTERS_VS), btp) {
         btp._3DCommandSubOpcode = binding_table_opcodes[s];
         btp.PointertoVSBindingTable = cmd_buffer->state.binding_tables[s].offset;
      }
   }
}

static struct anv_address
get_push_range_address(struct anv_cmd_buffer *cmd_buffer,
                       const struct anv_shader_bin *shader,
                       const struct anv_push_range *range)
{
   struct anv_cmd_graphics_state *gfx_state = &cmd_buffer->state.gfx;
   switch (range->set) {
   case ANV_DESCRIPTOR_SET_DESCRIPTORS: {
      /* This is a descriptor set buffer so the set index is
       * actually given by binding->binding.  (Yes, that's
       * confusing.)
       */
      struct anv_descriptor_set *set =
         gfx_state->base.descriptors[range->index];
      return anv_descriptor_set_address(set);
   }

   case ANV_DESCRIPTOR_SET_PUSH_CONSTANTS: {
      if (gfx_state->base.push_constants_state.alloc_size == 0) {
         gfx_state->base.push_constants_state =
            anv_cmd_buffer_gfx_push_constants(cmd_buffer);
      }
      return (struct anv_address) {
         .bo = cmd_buffer->device->dynamic_state_pool.block_pool.bo,
         .offset = gfx_state->base.push_constants_state.offset,
      };
   }

   case ANV_DESCRIPTOR_SET_SHADER_CONSTANTS:
      return (struct anv_address) {
         .bo = cmd_buffer->device->instruction_state_pool.block_pool.bo,
         .offset = shader->kernel.offset +
                   shader->prog_data->const_data_offset,
      };

   default: {
      assert(range->set < MAX_SETS);
      struct anv_descriptor_set *set =
         gfx_state->base.descriptors[range->set];
      const struct anv_descriptor *desc =
         &set->descriptors[range->index];

      if (desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
         if (desc->buffer_view)
            return desc->buffer_view->address;
      } else {
         assert(desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
         if (desc->buffer) {
            const struct anv_push_constants *push =
               &gfx_state->base.push_constants;
            uint32_t dynamic_offset =
               push->dynamic_offsets[range->dynamic_offset_index];
            return anv_address_add(desc->buffer->address,
                                   desc->offset + dynamic_offset);
         }
      }

      /* For NULL UBOs, we just return an address in the workaround BO.  We do
       * writes to it for workarounds but always at the bottom.  The higher
       * bytes should be all zeros.
       */
      assert(range->length * 32 <= 2048);
      return (struct anv_address) {
         .bo = cmd_buffer->device->workaround_bo,
         .offset = 1024,
      };
   }
   }
}


/** Returns the size in bytes of the bound buffer
 *
 * The range is relative to the start of the buffer, not the start of the
 * range.  The returned range may be smaller than
 *
 *    (range->start + range->length) * 32;
 */
static uint32_t
get_push_range_bound_size(struct anv_cmd_buffer *cmd_buffer,
                          const struct anv_shader_bin *shader,
                          const struct anv_push_range *range)
{
   assert(shader->stage != MESA_SHADER_COMPUTE);
   const struct anv_cmd_graphics_state *gfx_state = &cmd_buffer->state.gfx;
   switch (range->set) {
   case ANV_DESCRIPTOR_SET_DESCRIPTORS: {
      struct anv_descriptor_set *set =
         gfx_state->base.descriptors[range->index];
      assert(range->start * 32 < set->desc_mem.alloc_size);
      assert((range->start + range->length) * 32 <= set->desc_mem.alloc_size);
      return set->desc_mem.alloc_size;
   }

   case ANV_DESCRIPTOR_SET_PUSH_CONSTANTS:
      return (range->start + range->length) * 32;

   case ANV_DESCRIPTOR_SET_SHADER_CONSTANTS:
      return ALIGN(shader->prog_data->const_data_size, ANV_UBO_ALIGNMENT);

   default: {
      assert(range->set < MAX_SETS);
      struct anv_descriptor_set *set =
         gfx_state->base.descriptors[range->set];
      const struct anv_descriptor *desc =
         &set->descriptors[range->index];

      if (desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
         /* Here we promote a UBO to a binding table entry so that we can avoid a layer of indirection.
            * We use the descriptor set's internally allocated surface state to fill the binding table entry.
         */
         if (!desc->set_buffer_view)
            return 0;

         if (range->start * 32 > desc->set_buffer_view->range)
            return 0;

         return desc->set_buffer_view->range;
      } else {
         if (!desc->buffer)
            return 0;

         assert(desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
         /* Compute the offset within the buffer */
         const struct anv_push_constants *push =
            &gfx_state->base.push_constants;
         uint32_t dynamic_offset =
            push->dynamic_offsets[range->dynamic_offset_index];
         uint64_t offset = desc->offset + dynamic_offset;
         /* Clamp to the buffer size */
         offset = MIN2(offset, desc->buffer->vk.size);
         /* Clamp the range to the buffer size */
         uint32_t bound_range = MIN2(desc->range, desc->buffer->vk.size - offset);

         /* Align the range for consistency */
         bound_range = align(bound_range, ANV_UBO_ALIGNMENT);

         return bound_range;
      }
   }
   }
}

static void
cmd_buffer_emit_push_constant(struct anv_cmd_buffer *cmd_buffer,
                              gl_shader_stage stage,
                              struct anv_address *buffers,
                              unsigned buffer_count)
{
   const struct anv_cmd_graphics_state *gfx_state = &cmd_buffer->state.gfx;
   const struct anv_graphics_pipeline *pipeline = gfx_state->pipeline;

   static const uint32_t push_constant_opcodes[] = {
      [MESA_SHADER_VERTEX]                      = 21,
      [MESA_SHADER_TESS_CTRL]                   = 25, /* HS */
      [MESA_SHADER_TESS_EVAL]                   = 26, /* DS */
      [MESA_SHADER_GEOMETRY]                    = 22,
      [MESA_SHADER_FRAGMENT]                    = 23,
   };

   assert(stage < ARRAY_SIZE(push_constant_opcodes));

   UNUSED uint32_t mocs = anv_mocs(cmd_buffer->device, NULL, 0);

   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_CONSTANT_VS), c) {
      c._3DCommandSubOpcode = push_constant_opcodes[stage];

      /* Set MOCS, except on Gfx8, because the Broadwell PRM says:
       *
       *    "Constant Buffer Object Control State must be always
       *     programmed to zero."
       *
       * This restriction does not exist on any newer platforms.
       *
       * We only have one MOCS field for the whole packet, not one per
       * buffer.  We could go out of our way here to walk over all of
       * the buffers and see if any of them are used externally and use
       * the external MOCS.  However, the notion that someone would use
       * the same bit of memory for both scanout and a UBO is nuts.
       *
       * Let's not bother and assume it's all internal.
       */
#if GFX_VER != 8
      c.ConstantBody.MOCS = mocs;
#endif

      if (anv_pipeline_has_stage(pipeline, stage)) {
         const struct anv_pipeline_bind_map *bind_map =
            &pipeline->shaders[stage]->bind_map;

#if GFX_VERx10 >= 75
         /* The Skylake PRM contains the following restriction:
          *
          *    "The driver must ensure The following case does not occur
          *     without a flush to the 3D engine: 3DSTATE_CONSTANT_* with
          *     buffer 3 read length equal to zero committed followed by a
          *     3DSTATE_CONSTANT_* with buffer 0 read length not equal to
          *     zero committed."
          *
          * To avoid this, we program the buffers in the highest slots.
          * This way, slot 0 is only used if slot 3 is also used.
          */
         assert(buffer_count <= 4);
         const unsigned shift = 4 - buffer_count;
         for (unsigned i = 0; i < buffer_count; i++) {
            const struct anv_push_range *range = &bind_map->push_ranges[i];

            /* At this point we only have non-empty ranges */
            assert(range->length > 0);

            /* For Ivy Bridge, make sure we only set the first range (actual
             * push constants)
             */
            assert((GFX_VERx10 >= 75) || i == 0);

            c.ConstantBody.ReadLength[i + shift] = range->length;
            c.ConstantBody.Buffer[i + shift] =
               anv_address_add(buffers[i], range->start * 32);
         }
#else
         /* For Ivy Bridge, push constants are relative to dynamic state
          * base address and we only ever push actual push constants.
          */
         if (bind_map->push_ranges[0].length > 0) {
            assert(buffer_count == 1);
            assert(bind_map->push_ranges[0].set ==
                   ANV_DESCRIPTOR_SET_PUSH_CONSTANTS);
            assert(buffers[0].bo ==
                   cmd_buffer->device->dynamic_state_pool.block_pool.bo);
            c.ConstantBody.ReadLength[0] = bind_map->push_ranges[0].length;
            c.ConstantBody.Buffer[0].bo = NULL;
            c.ConstantBody.Buffer[0].offset = buffers[0].offset;
         }
         assert(bind_map->push_ranges[1].length == 0);
         assert(bind_map->push_ranges[2].length == 0);
         assert(bind_map->push_ranges[3].length == 0);
#endif
      }
   }
}

static void
cmd_buffer_flush_push_constants(struct anv_cmd_buffer *cmd_buffer,
                                VkShaderStageFlags dirty_stages)
{
   VkShaderStageFlags flushed = 0;
   struct anv_cmd_graphics_state *gfx_state = &cmd_buffer->state.gfx;
   const struct anv_graphics_pipeline *pipeline = gfx_state->pipeline;

   /* Compute robust pushed register access mask for each stage. */
   if (cmd_buffer->device->vk.enabled_features.robustBufferAccess) {
      anv_foreach_stage(stage, dirty_stages) {
         if (!anv_pipeline_has_stage(pipeline, stage))
            continue;

         const struct anv_shader_bin *shader = pipeline->shaders[stage];
         const struct anv_pipeline_bind_map *bind_map = &shader->bind_map;
         struct anv_push_constants *push = &gfx_state->base.push_constants;

         push->push_reg_mask[stage] = 0;
         /* Start of the current range in the shader, relative to the start of
          * push constants in the shader.
          */
         unsigned range_start_reg = 0;
         for (unsigned i = 0; i < 4; i++) {
            const struct anv_push_range *range = &bind_map->push_ranges[i];
            if (range->length == 0)
               continue;

            unsigned bound_size =
               get_push_range_bound_size(cmd_buffer, shader, range);
            if (bound_size >= range->start * 32) {
               unsigned bound_regs =
                  MIN2(DIV_ROUND_UP(bound_size, 32) - range->start,
                       range->length);
               assert(range_start_reg + bound_regs <= 64);
               push->push_reg_mask[stage] |= BITFIELD64_RANGE(range_start_reg,
                                                              bound_regs);
            }

            cmd_buffer->state.push_constants_dirty |=
               mesa_to_vk_shader_stage(stage);

            range_start_reg += range->length;
         }
      }
   }

   /* Resets the push constant state so that we allocate a new one if
    * needed.
    */
   gfx_state->base.push_constants_state = ANV_STATE_NULL;

   anv_foreach_stage(stage, dirty_stages) {
      unsigned buffer_count = 0;
      flushed |= mesa_to_vk_shader_stage(stage);
      UNUSED uint32_t max_push_range = 0;

      struct anv_address buffers[4] = {};
      if (anv_pipeline_has_stage(pipeline, stage)) {
         const struct anv_shader_bin *shader = pipeline->shaders[stage];
         const struct anv_pipeline_bind_map *bind_map = &shader->bind_map;

         /* We have to gather buffer addresses as a second step because the
          * loop above puts data into the push constant area and the call to
          * get_push_range_address is what locks our push constants and copies
          * them into the actual GPU buffer.  If we did the two loops at the
          * same time, we'd risk only having some of the sizes in the push
          * constant buffer when we did the copy.
          */
         for (unsigned i = 0; i < 4; i++) {
            const struct anv_push_range *range = &bind_map->push_ranges[i];
            if (range->length == 0)
               break;

            buffers[i] = get_push_range_address(cmd_buffer, shader, range);
            max_push_range = MAX2(max_push_range, range->length);
            buffer_count++;
         }

         /* We have at most 4 buffers but they should be tightly packed */
         for (unsigned i = buffer_count; i < 4; i++)
            assert(bind_map->push_ranges[i].length == 0);
      }

      cmd_buffer_emit_push_constant(cmd_buffer, stage, buffers, buffer_count);
   }

   cmd_buffer->state.push_constants_dirty &= ~flushed;
}

static void
cmd_buffer_emit_clip(struct anv_cmd_buffer *cmd_buffer)
{
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;

   if (!(cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) &&
       !BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY) &&
#if GFX_VER <= 7
       !BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_CULL_MODE) &&
       !BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_FRONT_FACE) &&
#endif
       !BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_VIEWPORT_COUNT))
      return;

   /* Take dynamic primitive topology in to account with
    *    3DSTATE_CLIP::ViewportXYClipTestEnable
    */
   VkPolygonMode dynamic_raster_mode =
      genX(raster_polygon_mode)(cmd_buffer->state.gfx.pipeline,
                                dyn->ia.primitive_topology);
   bool xy_clip_test_enable = (dynamic_raster_mode == VK_POLYGON_MODE_FILL);

   struct GENX(3DSTATE_CLIP) clip = {
      GENX(3DSTATE_CLIP_header),
#if GFX_VER <= 7
      .FrontWinding = genX(vk_to_intel_front_face)[dyn->rs.front_face],
      .CullMode     = genX(vk_to_intel_cullmode)[dyn->rs.cull_mode],
#endif
      .ViewportXYClipTestEnable = xy_clip_test_enable,
   };
   uint32_t dwords[GENX(3DSTATE_CLIP_length)];

   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   if (anv_pipeline_is_primitive(pipeline)) {
      const struct brw_vue_prog_data *last =
         anv_pipeline_get_last_vue_prog_data(pipeline);
      if (last->vue_map.slots_valid & VARYING_BIT_VIEWPORT) {
         clip.MaximumVPIndex = dyn->vp.viewport_count > 0 ?
                               dyn->vp.viewport_count - 1 : 0;
      }
   }

   GENX(3DSTATE_CLIP_pack)(NULL, dwords, &clip);
   anv_batch_emit_merge(&cmd_buffer->batch, dwords,
                        pipeline->gfx7.clip);
}

static void
cmd_buffer_emit_viewport(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_instance *instance = cmd_buffer->device->physical->instance;
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   uint32_t count = dyn->vp.viewport_count;
   const VkViewport *viewports = dyn->vp.viewports;
   struct anv_state sf_clip_state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, count * 64, 64);

   bool negative_one_to_one =
      cmd_buffer->state.gfx.pipeline->negative_one_to_one;

   float scale = negative_one_to_one ? 0.5f : 1.0f;

   for (uint32_t i = 0; i < count; i++) {
      const VkViewport *vp = &viewports[i];

      /* The gfx7 state struct has just the matrix and guardband fields, the
       * gfx8 struct adds the min/max viewport fields. */
      struct GENX(SF_CLIP_VIEWPORT) sfv = {
         .ViewportMatrixElementm00 = vp->width / 2,
         .ViewportMatrixElementm11 = vp->height / 2,
         .ViewportMatrixElementm22 = (vp->maxDepth - vp->minDepth) * scale,
         .ViewportMatrixElementm30 = vp->x + vp->width / 2,
         .ViewportMatrixElementm31 = vp->y + vp->height / 2,
         .ViewportMatrixElementm32 = negative_one_to_one ?
            (vp->minDepth + vp->maxDepth) * scale : vp->minDepth,
         .XMinClipGuardband = -1.0f,
         .XMaxClipGuardband = 1.0f,
         .YMinClipGuardband = -1.0f,
         .YMaxClipGuardband = 1.0f,
#if GFX_VER >= 8
         .XMinViewPort = vp->x,
         .XMaxViewPort = vp->x + vp->width - 1,
         .YMinViewPort = MIN2(vp->y, vp->y + vp->height),
         .YMaxViewPort = MAX2(vp->y, vp->y + vp->height) - 1,
#endif
      };

      /* Fix depth test misrenderings by lowering translated depth range */
      if (instance->lower_depth_range_rate != 1.0f)
         sfv.ViewportMatrixElementm32 *= instance->lower_depth_range_rate;

      const uint32_t fb_size_max = 1 << 14;
      uint32_t x_min = 0, x_max = fb_size_max;
      uint32_t y_min = 0, y_max = fb_size_max;

      /* If we have a valid renderArea, include that */
      if (gfx->render_area.extent.width > 0 &&
          gfx->render_area.extent.height > 0) {
         x_min = MAX2(x_min, gfx->render_area.offset.x);
         x_max = MIN2(x_min, gfx->render_area.offset.x +
                             gfx->render_area.extent.width);
         y_min = MAX2(y_min, gfx->render_area.offset.y);
         y_max = MIN2(y_min, gfx->render_area.offset.y +
                             gfx->render_area.extent.height);
      }

      /* The client is required to have enough scissors for whatever it sets
       * as ViewportIndex but it's possible that they've got more viewports
       * set from a previous command.  Also, from the Vulkan 1.3.207:
       *
       *    "The application must ensure (using scissor if necessary) that
       *    all rendering is contained within the render area."
       *
       * If the client doesn't set a scissor, that basically means it
       * guarantees everything is in-bounds already.  If we end up using a
       * guardband of [-1, 1] in that case, there shouldn't be much loss.
       * It's theoretically possible that they could do all their clipping
       * with clip planes but that'd be a bit odd.
       */
      if (i < dyn->vp.scissor_count) {
         const VkRect2D *scissor = &dyn->vp.scissors[i];
         x_min = MAX2(x_min, scissor->offset.x);
         x_max = MIN2(x_min, scissor->offset.x + scissor->extent.width);
         y_min = MAX2(y_min, scissor->offset.y);
         y_max = MIN2(y_min, scissor->offset.y + scissor->extent.height);
      }

      /* Only bother calculating the guardband if our known render area is
       * less than the maximum size.  Otherwise, it will calculate [-1, 1]
       * anyway but possibly with precision loss.
       */
      if (x_min > 0 || x_max < fb_size_max ||
          y_min > 0 || y_max < fb_size_max) {
         intel_calculate_guardband_size(x_min, x_max, y_min, y_max,
                                        sfv.ViewportMatrixElementm00,
                                        sfv.ViewportMatrixElementm11,
                                        sfv.ViewportMatrixElementm30,
                                        sfv.ViewportMatrixElementm31,
                                        &sfv.XMinClipGuardband,
                                        &sfv.XMaxClipGuardband,
                                        &sfv.YMinClipGuardband,
                                        &sfv.YMaxClipGuardband);
      }

      GENX(SF_CLIP_VIEWPORT_pack)(NULL, sf_clip_state.map + i * 64, &sfv);
   }

   anv_batch_emit(&cmd_buffer->batch,
                  GENX(3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP), clip) {
      clip.SFClipViewportPointer = sf_clip_state.offset;
   }
}

static void
cmd_buffer_emit_depth_viewport(struct anv_cmd_buffer *cmd_buffer,
                               bool depth_clamp_enable)
{
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   uint32_t count = dyn->vp.viewport_count;
   const VkViewport *viewports = dyn->vp.viewports;
   struct anv_state cc_state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, count * 8, 32);

   for (uint32_t i = 0; i < count; i++) {
      const VkViewport *vp = &viewports[i];

      /* From the Vulkan spec:
       *
       *    "It is valid for minDepth to be greater than or equal to
       *    maxDepth."
       */
      float min_depth = MIN2(vp->minDepth, vp->maxDepth);
      float max_depth = MAX2(vp->minDepth, vp->maxDepth);

      struct GENX(CC_VIEWPORT) cc_viewport = {
         .MinimumDepth = depth_clamp_enable ? min_depth : 0.0f,
         .MaximumDepth = depth_clamp_enable ? max_depth : 1.0f,
      };

      GENX(CC_VIEWPORT_pack)(NULL, cc_state.map + i * 8, &cc_viewport);
   }

   anv_batch_emit(&cmd_buffer->batch,
                  GENX(3DSTATE_VIEWPORT_STATE_POINTERS_CC), cc) {
      cc.CCViewportPointer = cc_state.offset;
   }
}

static void
cmd_buffer_emit_scissor(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   uint32_t count = dyn->vp.scissor_count;
   const VkRect2D *scissors = dyn->vp.scissors;
   const VkViewport *viewports = dyn->vp.viewports;

   /* Wa_1409725701:
    *    "The viewport-specific state used by the SF unit (SCISSOR_RECT) is
    *    stored as an array of up to 16 elements. The location of first
    *    element of the array, as specified by Pointer to SCISSOR_RECT, should
    *    be aligned to a 64-byte boundary.
    */
   uint32_t alignment = 64;
   struct anv_state scissor_state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, count * 8, alignment);

   for (uint32_t i = 0; i < count; i++) {
      const VkRect2D *s = &scissors[i];
      const VkViewport *vp = &viewports[i];

      /* Since xmax and ymax are inclusive, we have to have xmax < xmin or
       * ymax < ymin for empty clips.  In case clip x, y, width height are all
       * 0, the clamps below produce 0 for xmin, ymin, xmax, ymax, which isn't
       * what we want. Just special case empty clips and produce a canonical
       * empty clip. */
      static const struct GENX(SCISSOR_RECT) empty_scissor = {
         .ScissorRectangleYMin = 1,
         .ScissorRectangleXMin = 1,
         .ScissorRectangleYMax = 0,
         .ScissorRectangleXMax = 0
      };

      const int max = 0xffff;

      uint32_t y_min = MAX2(s->offset.y, MIN2(vp->y, vp->y + vp->height));
      uint32_t x_min = MAX2(s->offset.x, vp->x);
      int64_t y_max = MIN2(s->offset.y + s->extent.height - 1,
                       MAX2(vp->y, vp->y + vp->height) - 1);
      int64_t x_max = MIN2(s->offset.x + s->extent.width - 1,
                       vp->x + vp->width - 1);

      y_max = CLAMP(y_max, 0, INT16_MAX >> 1);
      x_max = CLAMP(x_max, 0, INT16_MAX >> 1);

      /* Do this math using int64_t so overflow gets clamped correctly. */
      if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
         y_min = CLAMP((uint64_t) y_min, gfx->render_area.offset.y, max);
         x_min = CLAMP((uint64_t) x_min, gfx->render_area.offset.x, max);
         y_max = CLAMP((uint64_t) y_max, 0,
                             gfx->render_area.offset.y +
                             gfx->render_area.extent.height - 1);
         x_max = CLAMP((uint64_t) x_max, 0,
                             gfx->render_area.offset.x +
                             gfx->render_area.extent.width - 1);
      }

      struct GENX(SCISSOR_RECT) scissor = {
         .ScissorRectangleYMin = y_min,
         .ScissorRectangleXMin = x_min,
         .ScissorRectangleYMax = y_max,
         .ScissorRectangleXMax = x_max
      };

      if (s->extent.width <= 0 || s->extent.height <= 0) {
         GENX(SCISSOR_RECT_pack)(NULL, scissor_state.map + i * 8,
                                 &empty_scissor);
      } else {
         GENX(SCISSOR_RECT_pack)(NULL, scissor_state.map + i * 8, &scissor);
      }
   }

   anv_batch_emit(&cmd_buffer->batch,
                  GENX(3DSTATE_SCISSOR_STATE_POINTERS), ssp) {
      ssp.ScissorRectPointer = scissor_state.offset;
   }
}

static void
cmd_buffer_emit_streamout(struct anv_cmd_buffer *cmd_buffer)
{
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;

#if GFX_VER == 7
#  define streamout_state_dw pipeline->gfx7.streamout_state
#else
#  define streamout_state_dw pipeline->gfx8.streamout_state
#endif

   uint32_t dwords[GENX(3DSTATE_STREAMOUT_length)];

   struct GENX(3DSTATE_STREAMOUT) so = {
      GENX(3DSTATE_STREAMOUT_header),
      .RenderingDisable = dyn->rs.rasterizer_discard_enable,
   };
   GENX(3DSTATE_STREAMOUT_pack)(NULL, dwords, &so);
   anv_batch_emit_merge(&cmd_buffer->batch, dwords, streamout_state_dw);
}

ALWAYS_INLINE static void
genX(cmd_buffer_flush_gfx_state)(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   uint32_t *p;

   assert((pipeline->active_stages & VK_SHADER_STAGE_COMPUTE_BIT) == 0);

   genX(cmd_buffer_config_l3)(cmd_buffer, pipeline->base.l3_config);

   genX(flush_pipeline_select_3d)(cmd_buffer);

   /* Apply any pending pipeline flushes we may have.  We want to apply them
    * now because, if any of those flushes are for things like push constants,
    * the GPU will read the state at weird times.
    */
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   uint32_t vb_emit = cmd_buffer->state.gfx.vb_dirty & pipeline->vb_used;
   if (cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE)
      vb_emit |= pipeline->vb_used;

   if (vb_emit) {
      const uint32_t num_buffers = __builtin_popcount(vb_emit);
      const uint32_t num_dwords = 1 + num_buffers * 4;

      p = anv_batch_emitn(&cmd_buffer->batch, num_dwords,
                          GENX(3DSTATE_VERTEX_BUFFERS));
      uint32_t i = 0;
      u_foreach_bit(vb, vb_emit) {
         struct anv_buffer *buffer = cmd_buffer->state.vertex_bindings[vb].buffer;
         uint32_t offset = cmd_buffer->state.vertex_bindings[vb].offset;

         struct GENX(VERTEX_BUFFER_STATE) state;
         if (buffer) {
            uint32_t stride = dyn->vi_binding_strides[vb];
            UNUSED uint32_t size = cmd_buffer->state.vertex_bindings[vb].size;

#if GFX_VER <= 7
            bool per_instance = pipeline->vb[vb].instanced;
            uint32_t divisor = pipeline->vb[vb].instance_divisor *
                               pipeline->instance_multiplier;
#endif

            state = (struct GENX(VERTEX_BUFFER_STATE)) {
               .VertexBufferIndex = vb,

               .MOCS = anv_mocs(cmd_buffer->device, buffer->address.bo,
                                ISL_SURF_USAGE_VERTEX_BUFFER_BIT),
#if GFX_VER <= 7
               .BufferAccessType = per_instance ? INSTANCEDATA : VERTEXDATA,
               .InstanceDataStepRate = per_instance ? divisor : 1,
#endif
               .AddressModifyEnable = true,
               .BufferPitch = stride,
               .BufferStartingAddress = anv_address_add(buffer->address, offset),
               .NullVertexBuffer = offset >= buffer->vk.size,

#if GFX_VER >= 8
               .BufferSize = size,
#else
               /* XXX: to handle dynamic offset for older gens we might want
                * to modify Endaddress, but there are issues when doing so:
                *
                * https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/7439
                */
               .EndAddress = anv_address_add(buffer->address, buffer->vk.size - 1),
#endif
            };
         } else {
            state = (struct GENX(VERTEX_BUFFER_STATE)) {
               .VertexBufferIndex = vb,
               .NullVertexBuffer = true,
               .MOCS = anv_mocs(cmd_buffer->device, NULL,
                                ISL_SURF_USAGE_VERTEX_BUFFER_BIT),
            };
         }

#if GFX_VER == 8
         genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer, vb,
                                                        state.BufferStartingAddress,
                                                        state.BufferSize);
#endif

         GENX(VERTEX_BUFFER_STATE_pack)(&cmd_buffer->batch, &p[1 + i * 4], &state);
         i++;
      }
   }

   cmd_buffer->state.gfx.vb_dirty &= ~vb_emit;

   uint32_t descriptors_dirty = cmd_buffer->state.descriptors_dirty &
                                pipeline->active_stages;
   if (!cmd_buffer->state.gfx.dirty && !descriptors_dirty &&
       !vk_dynamic_graphics_state_any_dirty(dyn) &&
       !cmd_buffer->state.push_constants_dirty)
      return;

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_XFB_ENABLE) ||
       (GFX_VER == 7 && (cmd_buffer->state.gfx.dirty &
                         ANV_CMD_DIRTY_PIPELINE))) {
      /* Wa_16011411144:
       *
       * SW must insert a PIPE_CONTROL cmd before and after the
       * 3dstate_so_buffer_index_0/1/2/3 states to ensure so_buffer_index_*
       * state is not combined with other state changes.
       */
      if (intel_device_info_is_dg2(cmd_buffer->device->info)) {
         anv_add_pending_pipe_bits(cmd_buffer,
                                   ANV_PIPE_CS_STALL_BIT,
                                   "before SO_BUFFER change WA");
         genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
      }

      /* We don't need any per-buffer dirty tracking because you're not
       * allowed to bind different XFB buffers while XFB is enabled.
       */
      for (unsigned idx = 0; idx < MAX_XFB_BUFFERS; idx++) {
         struct anv_xfb_binding *xfb = &cmd_buffer->state.xfb_bindings[idx];
         anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_SO_BUFFER), sob) {
            sob.SOBufferIndex = idx;

            if (cmd_buffer->state.xfb_enabled && xfb->buffer && xfb->size != 0) {
               sob.MOCS = anv_mocs(cmd_buffer->device, xfb->buffer->address.bo,
                                   ISL_SURF_USAGE_STREAM_OUT_BIT);
               sob.SurfaceBaseAddress = anv_address_add(xfb->buffer->address,
                                                        xfb->offset);
#if GFX_VER >= 8
               sob.SOBufferEnable = true;
               sob.StreamOffsetWriteEnable = false;
               /* Size is in DWords - 1 */
               sob.SurfaceSize = DIV_ROUND_UP(xfb->size, 4) - 1;
#else
               /* We don't have SOBufferEnable in 3DSTATE_SO_BUFFER on Gfx7 so
                * we trust in SurfaceEndAddress = SurfaceBaseAddress = 0 (the
                * default for an empty SO_BUFFER packet) to disable them.
                */
               sob.SurfacePitch = pipeline->gfx7.xfb_bo_pitch[idx];
               sob.SurfaceEndAddress = anv_address_add(xfb->buffer->address,
                                                       xfb->offset + xfb->size);
#endif
            } else {
               sob.MOCS = anv_mocs(cmd_buffer->device, NULL, 0);
            }
         }
      }
   }

   if (cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) {
      anv_batch_emit_batch(&cmd_buffer->batch, &pipeline->base.batch);

      /* If the pipeline changed, we may need to re-allocate push constant
       * space in the URB.
       */
      cmd_buffer_alloc_push_constants(cmd_buffer);
   }

#if GFX_VER <= 7
   if (cmd_buffer->state.descriptors_dirty & VK_SHADER_STAGE_VERTEX_BIT ||
       cmd_buffer->state.push_constants_dirty & VK_SHADER_STAGE_VERTEX_BIT) {
      /* From the IVB PRM Vol. 2, Part 1, Section 3.2.1:
       *
       *    "A PIPE_CONTROL with Post-Sync Operation set to 1h and a depth
       *    stall needs to be sent just prior to any 3DSTATE_VS,
       *    3DSTATE_URB_VS, 3DSTATE_CONSTANT_VS,
       *    3DSTATE_BINDING_TABLE_POINTER_VS,
       *    3DSTATE_SAMPLER_STATE_POINTER_VS command.  Only one
       *    PIPE_CONTROL needs to be sent before any combination of VS
       *    associated 3DSTATE."
       */
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.DepthStallEnable  = true;
         pc.PostSyncOperation = WriteImmediateData;
         pc.Address           = cmd_buffer->device->workaround_address;
         anv_debug_dump_pc(pc);
      }
   }
#endif

   /* Render targets live in the same binding table as fragment descriptors */
   if (cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_RENDER_TARGETS)
      descriptors_dirty |= VK_SHADER_STAGE_FRAGMENT_BIT;

   /* We emit the binding tables and sampler tables first, then emit push
    * constants and then finally emit binding table and sampler table
    * pointers.  It has to happen in this order, since emitting the binding
    * tables may change the push constants (in case of storage images). After
    * emitting push constants, on SKL+ we have to emit the corresponding
    * 3DSTATE_BINDING_TABLE_POINTER_* for the push constants to take effect.
    */
   uint32_t dirty = 0;
   if (descriptors_dirty) {
      dirty = flush_descriptor_sets(cmd_buffer,
                                    &cmd_buffer->state.gfx.base,
                                    descriptors_dirty,
                                    pipeline->shaders,
                                    ARRAY_SIZE(pipeline->shaders));
      cmd_buffer->state.descriptors_dirty &= ~dirty;
   }

   if (dirty || cmd_buffer->state.push_constants_dirty) {
      /* Because we're pushing UBOs, we have to push whenever either
       * descriptors or push constants is dirty.
       */
      dirty |= cmd_buffer->state.push_constants_dirty;
      cmd_buffer_flush_push_constants(cmd_buffer,
                                      dirty & VK_SHADER_STAGE_ALL_GRAPHICS);
   }

   if (dirty & VK_SHADER_STAGE_ALL_GRAPHICS) {
      cmd_buffer_emit_descriptor_pointers(cmd_buffer,
                                          dirty & VK_SHADER_STAGE_ALL_GRAPHICS);
   }

   cmd_buffer_emit_clip(cmd_buffer);

   if ((cmd_buffer->state.gfx.dirty & (ANV_CMD_DIRTY_PIPELINE |
                                       ANV_CMD_DIRTY_XFB_ENABLE)) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_RASTERIZER_DISCARD_ENABLE))
      cmd_buffer_emit_streamout(cmd_buffer);

   if ((cmd_buffer->state.gfx.dirty & (ANV_CMD_DIRTY_PIPELINE |
                                       ANV_CMD_DIRTY_RENDER_TARGETS)) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_VIEWPORTS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_SCISSORS)) {
      cmd_buffer_emit_viewport(cmd_buffer);
      cmd_buffer_emit_depth_viewport(cmd_buffer,
                                     pipeline->depth_clamp_enable);
      cmd_buffer_emit_scissor(cmd_buffer);
   }

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY)) {
      uint32_t topology;
      if (anv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_EVAL))
         topology = _3DPRIM_PATCHLIST(pipeline->patch_control_points);
      else
         topology = genX(vk_to_intel_primitive_type)[dyn->ia.primitive_topology];

      cmd_buffer->state.gfx.primitive_topology = topology;

#if (GFX_VER >= 8)
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF_TOPOLOGY), vft) {
         vft.PrimitiveTopologyType = topology;
      }
#endif
   }

   genX(cmd_buffer_flush_dynamic_state)(cmd_buffer);
}

static void
emit_vertex_bo(struct anv_cmd_buffer *cmd_buffer,
               struct anv_address addr,
               uint32_t size, uint32_t index)
{
   uint32_t *p = anv_batch_emitn(&cmd_buffer->batch, 5,
                                 GENX(3DSTATE_VERTEX_BUFFERS));

   GENX(VERTEX_BUFFER_STATE_pack)(&cmd_buffer->batch, p + 1,
      &(struct GENX(VERTEX_BUFFER_STATE)) {
         .VertexBufferIndex = index,
         .AddressModifyEnable = true,
         .BufferPitch = 0,
         .MOCS = anv_mocs(cmd_buffer->device, addr.bo,
                          ISL_SURF_USAGE_VERTEX_BUFFER_BIT),
         .NullVertexBuffer = size == 0,
#if (GFX_VER >= 8)
         .BufferStartingAddress = addr,
         .BufferSize = size
#else
         .BufferStartingAddress = addr,
         .EndAddress = anv_address_add(addr, size),
#endif
      });

   genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer,
                                                  index, addr, size);
}

static void
emit_base_vertex_instance_bo(struct anv_cmd_buffer *cmd_buffer,
                             struct anv_address addr)
{
   emit_vertex_bo(cmd_buffer, addr, addr.bo ? 8 : 0, ANV_SVGS_VB_INDEX);
}

static void
emit_base_vertex_instance(struct anv_cmd_buffer *cmd_buffer,
                          uint32_t base_vertex, uint32_t base_instance)
{
   if (base_vertex == 0 && base_instance == 0) {
      emit_base_vertex_instance_bo(cmd_buffer, ANV_NULL_ADDRESS);
   } else {
      struct anv_state id_state =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, 8, 4);

      ((uint32_t *)id_state.map)[0] = base_vertex;
      ((uint32_t *)id_state.map)[1] = base_instance;

      struct anv_address addr = {
         .bo = cmd_buffer->device->dynamic_state_pool.block_pool.bo,
         .offset = id_state.offset,
      };

      emit_base_vertex_instance_bo(cmd_buffer, addr);
   }
}

static void
emit_draw_index(struct anv_cmd_buffer *cmd_buffer, uint32_t draw_index)
{
   struct anv_state state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, 4, 4);

   ((uint32_t *)state.map)[0] = draw_index;

   struct anv_address addr = {
      .bo = cmd_buffer->device->dynamic_state_pool.block_pool.bo,
      .offset = state.offset,
   };

   emit_vertex_bo(cmd_buffer, addr, 4, ANV_DRAWID_VB_INDEX);
}

static void
update_dirty_vbs_for_gfx8_vb_flush(struct anv_cmd_buffer *cmd_buffer,
                                   uint32_t access_type)
{
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   uint64_t vb_used = pipeline->vb_used;
   if (vs_prog_data->uses_firstvertex ||
       vs_prog_data->uses_baseinstance)
      vb_used |= 1ull << ANV_SVGS_VB_INDEX;
   if (vs_prog_data->uses_drawid)
      vb_used |= 1ull << ANV_DRAWID_VB_INDEX;

   genX(cmd_buffer_update_dirty_vbs_for_gfx8_vb_flush)(cmd_buffer,
                                                       access_type == RANDOM,
                                                       vb_used);
}

ALWAYS_INLINE static void
cmd_buffer_emit_vertex_constants_and_flush(struct anv_cmd_buffer *cmd_buffer,
                                           const struct brw_vs_prog_data *vs_prog_data,
                                           uint32_t base_vertex,
                                           uint32_t base_instance,
                                           uint32_t draw_id,
                                           bool force_flush)
{
   bool emitted = false;
   if (vs_prog_data->uses_firstvertex ||
       vs_prog_data->uses_baseinstance) {
      emit_base_vertex_instance(cmd_buffer, base_vertex, base_instance);
      emitted = true;
   }
   if (vs_prog_data->uses_drawid) {
      emit_draw_index(cmd_buffer, draw_id);
      emitted = true;
   }
   /* Emitting draw index or vertex index BOs may result in needing
    * additional VF cache flushes.
    */
   if (emitted || force_flush)
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
}

void genX(CmdDraw)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   const uint32_t count =
      vertexCount * instanceCount * pipeline->instance_multiplier;
   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw", count);
   trace_intel_begin_draw(&cmd_buffer->trace);

   /* Select pipeline here to allow
    * cmd_buffer_emit_vertex_constants_and_flush() without flushing before
    * cmd_buffer_flush_gfx_state().
    */
   genX(flush_pipeline_select_3d)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   cmd_buffer_emit_vertex_constants_and_flush(cmd_buffer, vs_prog_data,
                                              firstVertex, firstInstance, 0,
                                              false /* force_flush */);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
      prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
      prim.VertexAccessType         = SEQUENTIAL;
      prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
      prim.VertexCountPerInstance   = vertexCount;
      prim.StartVertexLocation      = firstVertex;
      prim.InstanceCount            = instanceCount *
                                      pipeline->instance_multiplier;
      prim.StartInstanceLocation    = firstInstance;
      prim.BaseVertexLocation       = 0;
   }

   update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, SEQUENTIAL);

   trace_intel_end_draw(&cmd_buffer->trace, count);
}

void genX(CmdDrawMultiEXT)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawInfoEXT                   *pVertexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   const uint32_t count =
      drawCount * instanceCount * pipeline->instance_multiplier;
   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw_multi", count);
   trace_intel_begin_draw_multi(&cmd_buffer->trace);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   uint32_t i = 0;
   vk_foreach_multi_draw(draw, i, pVertexInfo, drawCount, stride) {
      cmd_buffer_emit_vertex_constants_and_flush(cmd_buffer, vs_prog_data,
                                                 draw->firstVertex,
                                                 firstInstance, i, !i);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
         prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
         prim.VertexAccessType         = SEQUENTIAL;
         prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
         prim.VertexCountPerInstance   = draw->vertexCount;
         prim.StartVertexLocation      = draw->firstVertex;
         prim.InstanceCount            = instanceCount *
                                         pipeline->instance_multiplier;
         prim.StartInstanceLocation    = firstInstance;
         prim.BaseVertexLocation       = 0;
      }
   }

   update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, SEQUENTIAL);

   trace_intel_end_draw_multi(&cmd_buffer->trace, count);
}

void genX(CmdDrawIndexed)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   const uint32_t count =
      indexCount * instanceCount * pipeline->instance_multiplier;
   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw indexed",
                        count);
   trace_intel_begin_draw_indexed(&cmd_buffer->trace);

   /* Select pipeline here to allow
    * cmd_buffer_emit_vertex_constants_and_flush() without flushing before
    * cmd_buffer_flush_gfx_state().
    */
   genX(flush_pipeline_select_3d)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   cmd_buffer_emit_vertex_constants_and_flush(cmd_buffer, vs_prog_data,
                                              vertexOffset, firstInstance,
                                              0, false /* force_flush */);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
      prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
      prim.VertexAccessType         = RANDOM;
      prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
      prim.VertexCountPerInstance   = indexCount;
      prim.StartVertexLocation      = firstIndex;
      prim.InstanceCount            = instanceCount *
                                      pipeline->instance_multiplier;
      prim.StartInstanceLocation    = firstInstance;
      prim.BaseVertexLocation       = vertexOffset;
   }

   update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, RANDOM);

   trace_intel_end_draw_indexed(&cmd_buffer->trace, count);
}

void genX(CmdDrawMultiIndexedEXT)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawIndexedInfoEXT            *pIndexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride,
    const int32_t                              *pVertexOffset)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   const uint32_t count =
      drawCount * instanceCount * pipeline->instance_multiplier;
   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw indexed_multi",
                        count);
   trace_intel_begin_draw_indexed_multi(&cmd_buffer->trace);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   uint32_t i = 0;
   if (pVertexOffset) {
      if (vs_prog_data->uses_drawid) {
         bool emitted = true;
         if (vs_prog_data->uses_firstvertex ||
             vs_prog_data->uses_baseinstance) {
            emit_base_vertex_instance(cmd_buffer, *pVertexOffset, firstInstance);
            emitted = true;
         }
         vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
            if (vs_prog_data->uses_drawid) {
               emit_draw_index(cmd_buffer, i);
               emitted = true;
            }
            /* Emitting draw index or vertex index BOs may result in needing
             * additional VF cache flushes.
             */
            if (emitted)
               genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

            anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
               prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
               prim.VertexAccessType         = RANDOM;
               prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
               prim.VertexCountPerInstance   = draw->indexCount;
               prim.StartVertexLocation      = draw->firstIndex;
               prim.InstanceCount            = instanceCount *
                                               pipeline->instance_multiplier;
               prim.StartInstanceLocation    = firstInstance;
               prim.BaseVertexLocation       = *pVertexOffset;
            }
            emitted = false;
         }
      } else {
         if (vs_prog_data->uses_firstvertex ||
             vs_prog_data->uses_baseinstance) {
            emit_base_vertex_instance(cmd_buffer, *pVertexOffset, firstInstance);
            /* Emitting draw index or vertex index BOs may result in needing
             * additional VF cache flushes.
             */
            genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
         }
         vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
            anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
               prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
               prim.VertexAccessType         = RANDOM;
               prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
               prim.VertexCountPerInstance   = draw->indexCount;
               prim.StartVertexLocation      = draw->firstIndex;
               prim.InstanceCount            = instanceCount *
                                               pipeline->instance_multiplier;
               prim.StartInstanceLocation    = firstInstance;
               prim.BaseVertexLocation       = *pVertexOffset;
            }
         }
      }
   } else {
      vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
         cmd_buffer_emit_vertex_constants_and_flush(cmd_buffer, vs_prog_data,
                                                    draw->vertexOffset,
                                                    firstInstance, i, i != 0);

         anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
            prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
            prim.VertexAccessType         = RANDOM;
            prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
            prim.VertexCountPerInstance   = draw->indexCount;
            prim.StartVertexLocation      = draw->firstIndex;
            prim.InstanceCount            = instanceCount *
                                            pipeline->instance_multiplier;
            prim.StartInstanceLocation    = firstInstance;
            prim.BaseVertexLocation       = draw->vertexOffset;
         }
      }
   }

   update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, RANDOM);

   trace_intel_end_draw_indexed_multi(&cmd_buffer->trace, count);
}

/* Auto-Draw / Indirect Registers */
#define GFX7_3DPRIM_END_OFFSET          0x2420
#define GFX7_3DPRIM_START_VERTEX        0x2430
#define GFX7_3DPRIM_VERTEX_COUNT        0x2434
#define GFX7_3DPRIM_INSTANCE_COUNT      0x2438
#define GFX7_3DPRIM_START_INSTANCE      0x243C
#define GFX7_3DPRIM_BASE_VERTEX         0x2440

void genX(CmdDrawIndirectByteCountEXT)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride)
{
#if GFX_VERx10 >= 75
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, counter_buffer, counterBuffer);
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   /* firstVertex is always zero for this draw function */
   const uint32_t firstVertex = 0;

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw indirect byte count",
                        instanceCount * pipeline->instance_multiplier);
   trace_intel_begin_draw_indirect_byte_count(&cmd_buffer->trace);

   /* Select pipeline here to allow
    * cmd_buffer_emit_vertex_constants_and_flush() without flushing before
    * emit_base_vertex_instance() & emit_draw_index().
    */
   genX(flush_pipeline_select_3d)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   if (vs_prog_data->uses_firstvertex ||
       vs_prog_data->uses_baseinstance)
      emit_base_vertex_instance(cmd_buffer, firstVertex, firstInstance);
   if (vs_prog_data->uses_drawid)
      emit_draw_index(cmd_buffer, 0);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);
   struct mi_value count =
      mi_mem32(anv_address_add(counter_buffer->address,
                                   counterBufferOffset));
   if (counterOffset)
      count = mi_isub(&b, count, mi_imm(counterOffset));
   count = mi_udiv32_imm(&b, count, vertexStride);
   mi_store(&b, mi_reg32(GFX7_3DPRIM_VERTEX_COUNT), count);

   mi_store(&b, mi_reg32(GFX7_3DPRIM_START_VERTEX), mi_imm(firstVertex));
   mi_store(&b, mi_reg32(GFX7_3DPRIM_INSTANCE_COUNT),
            mi_imm(instanceCount * pipeline->instance_multiplier));
   mi_store(&b, mi_reg32(GFX7_3DPRIM_START_INSTANCE), mi_imm(firstInstance));
   mi_store(&b, mi_reg32(GFX7_3DPRIM_BASE_VERTEX), mi_imm(0));

   anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
      prim.IndirectParameterEnable  = true;
      prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
      prim.VertexAccessType         = SEQUENTIAL;
      prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
   }

   update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, SEQUENTIAL);

   trace_intel_end_draw_indirect_byte_count(&cmd_buffer->trace,
      instanceCount * pipeline->instance_multiplier);
#endif /* GFX_VERx10 >= 75 */
}

static void
load_indirect_parameters(struct anv_cmd_buffer *cmd_buffer,
                         struct anv_address addr,
                         bool indexed)
{
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   mi_store(&b, mi_reg32(GFX7_3DPRIM_VERTEX_COUNT),
                mi_mem32(anv_address_add(addr, 0)));

   struct mi_value instance_count = mi_mem32(anv_address_add(addr, 4));
   if (pipeline->instance_multiplier > 1) {
#if GFX_VERx10 >= 75
      instance_count = mi_imul_imm(&b, instance_count,
                                   pipeline->instance_multiplier);
#else
      anv_finishme("Multiview + indirect draw requires MI_MATH; "
                   "MI_MATH is not supported on Ivy Bridge");
#endif
   }
   mi_store(&b, mi_reg32(GFX7_3DPRIM_INSTANCE_COUNT), instance_count);

   mi_store(&b, mi_reg32(GFX7_3DPRIM_START_VERTEX),
                mi_mem32(anv_address_add(addr, 8)));

   if (indexed) {
      mi_store(&b, mi_reg32(GFX7_3DPRIM_BASE_VERTEX),
                   mi_mem32(anv_address_add(addr, 12)));
      mi_store(&b, mi_reg32(GFX7_3DPRIM_START_INSTANCE),
                   mi_mem32(anv_address_add(addr, 16)));
   } else {
      mi_store(&b, mi_reg32(GFX7_3DPRIM_START_INSTANCE),
                   mi_mem32(anv_address_add(addr, 12)));
      mi_store(&b, mi_reg32(GFX7_3DPRIM_BASE_VERTEX), mi_imm(0));
   }
}

void genX(CmdDrawIndirect)(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    _buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, buffer, _buffer);
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw indirect",
                        drawCount);
   trace_intel_begin_draw_indirect(&cmd_buffer->trace);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   for (uint32_t i = 0; i < drawCount; i++) {
      struct anv_address draw = anv_address_add(buffer->address, offset);

      if (vs_prog_data->uses_firstvertex ||
          vs_prog_data->uses_baseinstance)
         emit_base_vertex_instance_bo(cmd_buffer, anv_address_add(draw, 8));
      if (vs_prog_data->uses_drawid)
         emit_draw_index(cmd_buffer, i);

      /* Emitting draw index or vertex index BOs may result in needing
       * additional VF cache flushes.
       */
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      load_indirect_parameters(cmd_buffer, draw, false);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
         prim.IndirectParameterEnable  = true;
         prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
         prim.VertexAccessType         = SEQUENTIAL;
         prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
      }

      update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, SEQUENTIAL);

      offset += stride;
   }

   trace_intel_end_draw_indirect(&cmd_buffer->trace, drawCount);
}

void genX(CmdDrawIndexedIndirect)(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    _buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, buffer, _buffer);
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw indexed indirect",
                        drawCount);
   trace_intel_begin_draw_indexed_indirect(&cmd_buffer->trace);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   for (uint32_t i = 0; i < drawCount; i++) {
      struct anv_address draw = anv_address_add(buffer->address, offset);

      /* TODO: We need to stomp base vertex to 0 somehow */
      if (vs_prog_data->uses_firstvertex ||
          vs_prog_data->uses_baseinstance)
         emit_base_vertex_instance_bo(cmd_buffer, anv_address_add(draw, 12));
      if (vs_prog_data->uses_drawid)
         emit_draw_index(cmd_buffer, i);

      /* Emitting draw index or vertex index BOs may result in needing
       * additional VF cache flushes.
       */
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      load_indirect_parameters(cmd_buffer, draw, true);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
         prim.IndirectParameterEnable  = true;
         prim.PredicateEnable          = cmd_buffer->state.conditional_render_enabled;
         prim.VertexAccessType         = RANDOM;
         prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
      }

      update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, RANDOM);

      offset += stride;
   }

   trace_intel_end_draw_indexed_indirect(&cmd_buffer->trace, drawCount);
}

static struct mi_value
prepare_for_draw_count_predicate(struct anv_cmd_buffer *cmd_buffer,
                                 struct mi_builder *b,
                                 struct anv_buffer *count_buffer,
                                 uint64_t countBufferOffset)
{
   struct anv_address count_address =
         anv_address_add(count_buffer->address, countBufferOffset);

   struct mi_value ret = mi_imm(0);

   if (cmd_buffer->state.conditional_render_enabled) {
#if GFX_VERx10 >= 75
      ret = mi_new_gpr(b);
      mi_store(b, mi_value_ref(b, ret), mi_mem32(count_address));
#endif
   } else {
      /* Upload the current draw count from the draw parameters buffer to
       * MI_PREDICATE_SRC0.
       */
      mi_store(b, mi_reg64(MI_PREDICATE_SRC0), mi_mem32(count_address));
      mi_store(b, mi_reg32(MI_PREDICATE_SRC1 + 4), mi_imm(0));
   }

   return ret;
}

static void
emit_draw_count_predicate(struct anv_cmd_buffer *cmd_buffer,
                          struct mi_builder *b,
                          uint32_t draw_index)
{
   /* Upload the index of the current primitive to MI_PREDICATE_SRC1. */
   mi_store(b, mi_reg32(MI_PREDICATE_SRC1), mi_imm(draw_index));

   if (draw_index == 0) {
      anv_batch_emit(&cmd_buffer->batch, GENX(MI_PREDICATE), mip) {
         mip.LoadOperation    = LOAD_LOADINV;
         mip.CombineOperation = COMBINE_SET;
         mip.CompareOperation = COMPARE_SRCS_EQUAL;
      }
   } else {
      /* While draw_index < draw_count the predicate's result will be
       *  (draw_index == draw_count) ^ TRUE = TRUE
       * When draw_index == draw_count the result is
       *  (TRUE) ^ TRUE = FALSE
       * After this all results will be:
       *  (FALSE) ^ FALSE = FALSE
       */
      anv_batch_emit(&cmd_buffer->batch, GENX(MI_PREDICATE), mip) {
         mip.LoadOperation    = LOAD_LOAD;
         mip.CombineOperation = COMBINE_XOR;
         mip.CompareOperation = COMPARE_SRCS_EQUAL;
      }
   }
}

#if GFX_VERx10 >= 75
static void
emit_draw_count_predicate_with_conditional_render(
                          struct anv_cmd_buffer *cmd_buffer,
                          struct mi_builder *b,
                          uint32_t draw_index,
                          struct mi_value max)
{
   struct mi_value pred = mi_ult(b, mi_imm(draw_index), max);
   pred = mi_iand(b, pred, mi_reg64(ANV_PREDICATE_RESULT_REG));

#if GFX_VER >= 8
   mi_store(b, mi_reg32(MI_PREDICATE_RESULT), pred);
#else
   /* MI_PREDICATE_RESULT is not whitelisted in i915 command parser
    * so we emit MI_PREDICATE to set it.
    */

   mi_store(b, mi_reg64(MI_PREDICATE_SRC0), pred);
   mi_store(b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(0));

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOADINV;
      mip.CombineOperation = COMBINE_SET;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }
#endif
}
#endif

static void
emit_draw_count_predicate_cond(struct anv_cmd_buffer *cmd_buffer,
                               struct mi_builder *b,
                               uint32_t draw_index,
                               struct mi_value max)
{
#if GFX_VERx10 >= 75
   if (cmd_buffer->state.conditional_render_enabled) {
      emit_draw_count_predicate_with_conditional_render(
            cmd_buffer, b, draw_index, mi_value_ref(b, max));
   } else {
      emit_draw_count_predicate(cmd_buffer, b, draw_index);
   }
#else
   emit_draw_count_predicate(cmd_buffer, b, draw_index);
#endif
}

void genX(CmdDrawIndirectCount)(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    _buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    _countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, buffer, _buffer);
   ANV_FROM_HANDLE(anv_buffer, count_buffer, _countBuffer);
   struct anv_cmd_state *cmd_state = &cmd_buffer->state;
   struct anv_graphics_pipeline *pipeline = cmd_state->gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw indirect count",
                        0);
   trace_intel_begin_draw_indirect_count(&cmd_buffer->trace);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);
   struct mi_value max =
      prepare_for_draw_count_predicate(cmd_buffer, &b,
                                       count_buffer, countBufferOffset);

   for (uint32_t i = 0; i < maxDrawCount; i++) {
      struct anv_address draw = anv_address_add(buffer->address, offset);

      emit_draw_count_predicate_cond(cmd_buffer, &b, i, max);

      if (vs_prog_data->uses_firstvertex ||
          vs_prog_data->uses_baseinstance)
         emit_base_vertex_instance_bo(cmd_buffer, anv_address_add(draw, 8));
      if (vs_prog_data->uses_drawid)
         emit_draw_index(cmd_buffer, i);

      /* Emitting draw index or vertex index BOs may result in needing
       * additional VF cache flushes.
       */
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      load_indirect_parameters(cmd_buffer, draw, false);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
         prim.IndirectParameterEnable  = true;
         prim.PredicateEnable          = true;
         prim.VertexAccessType         = SEQUENTIAL;
         prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
      }

      update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, SEQUENTIAL);

      offset += stride;
   }

   mi_value_unref(&b, max);

   trace_intel_end_draw_indirect_count(&cmd_buffer->trace, maxDrawCount);
}

void genX(CmdDrawIndexedIndirectCount)(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    _buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    _countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, buffer, _buffer);
   ANV_FROM_HANDLE(anv_buffer, count_buffer, _countBuffer);
   struct anv_cmd_state *cmd_state = &cmd_buffer->state;
   struct anv_graphics_pipeline *pipeline = cmd_state->gfx.pipeline;
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_DRAW,
                        "draw indexed indirect count",
                        0);
   trace_intel_begin_draw_indexed_indirect_count(&cmd_buffer->trace);

   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);
   struct mi_value max =
      prepare_for_draw_count_predicate(cmd_buffer, &b,
                                       count_buffer, countBufferOffset);

   for (uint32_t i = 0; i < maxDrawCount; i++) {
      struct anv_address draw = anv_address_add(buffer->address, offset);

      emit_draw_count_predicate_cond(cmd_buffer, &b, i, max);

      /* TODO: We need to stomp base vertex to 0 somehow */
      if (vs_prog_data->uses_firstvertex ||
          vs_prog_data->uses_baseinstance)
         emit_base_vertex_instance_bo(cmd_buffer, anv_address_add(draw, 12));
      if (vs_prog_data->uses_drawid)
         emit_draw_index(cmd_buffer, i);

      /* Emitting draw index or vertex index BOs may result in needing
       * additional VF cache flushes.
       */
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      load_indirect_parameters(cmd_buffer, draw, true);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
         prim.IndirectParameterEnable  = true;
         prim.PredicateEnable          = true;
         prim.VertexAccessType         = RANDOM;
         prim.PrimitiveTopologyType    = cmd_buffer->state.gfx.primitive_topology;
      }

      update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, RANDOM);

      offset += stride;
   }

   mi_value_unref(&b, max);

   trace_intel_end_draw_indexed_indirect_count(&cmd_buffer->trace, maxDrawCount);

}

void genX(CmdBeginTransformFeedbackEXT)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   assert(firstCounterBuffer < MAX_XFB_BUFFERS);
   assert(counterBufferCount <= MAX_XFB_BUFFERS);
   assert(firstCounterBuffer + counterBufferCount <= MAX_XFB_BUFFERS);

   /* From the SKL PRM Vol. 2c, SO_WRITE_OFFSET:
    *
    *    "Ssoftware must ensure that no HW stream output operations can be in
    *    process or otherwise pending at the point that the MI_LOAD/STORE
    *    commands are processed. This will likely require a pipeline flush."
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_CS_STALL_BIT,
                             "begin transform feedback");
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   for (uint32_t idx = 0; idx < MAX_XFB_BUFFERS; idx++) {
      /* If we have a counter buffer, this is a resume so we need to load the
       * value into the streamout offset register.  Otherwise, this is a begin
       * and we need to reset it to zero.
       */
      if (pCounterBuffers &&
          idx >= firstCounterBuffer &&
          idx - firstCounterBuffer < counterBufferCount &&
          pCounterBuffers[idx - firstCounterBuffer] != VK_NULL_HANDLE) {
         uint32_t cb_idx = idx - firstCounterBuffer;
         ANV_FROM_HANDLE(anv_buffer, counter_buffer, pCounterBuffers[cb_idx]);
         uint64_t offset = pCounterBufferOffsets ?
                           pCounterBufferOffsets[cb_idx] : 0;

         anv_batch_emit(&cmd_buffer->batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
            lrm.RegisterAddress  = GENX(SO_WRITE_OFFSET0_num) + idx * 4;
            lrm.MemoryAddress    = anv_address_add(counter_buffer->address,
                                                   offset);
         }
      } else {
         anv_batch_emit(&cmd_buffer->batch, GENX(MI_LOAD_REGISTER_IMM), lri) {
            lri.RegisterOffset   = GENX(SO_WRITE_OFFSET0_num) + idx * 4;
            lri.DataDWord        = 0;
         }
      }
   }

   cmd_buffer->state.xfb_enabled = true;
   cmd_buffer->state.gfx.dirty |= ANV_CMD_DIRTY_XFB_ENABLE;
}

void genX(CmdEndTransformFeedbackEXT)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   assert(firstCounterBuffer < MAX_XFB_BUFFERS);
   assert(counterBufferCount <= MAX_XFB_BUFFERS);
   assert(firstCounterBuffer + counterBufferCount <= MAX_XFB_BUFFERS);

   /* From the SKL PRM Vol. 2c, SO_WRITE_OFFSET:
    *
    *    "Ssoftware must ensure that no HW stream output operations can be in
    *    process or otherwise pending at the point that the MI_LOAD/STORE
    *    commands are processed. This will likely require a pipeline flush."
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_CS_STALL_BIT,
                             "end transform feedback");
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   for (uint32_t cb_idx = 0; cb_idx < counterBufferCount; cb_idx++) {
      unsigned idx = firstCounterBuffer + cb_idx;

      /* If we have a counter buffer, this is a resume so we need to load the
       * value into the streamout offset register.  Otherwise, this is a begin
       * and we need to reset it to zero.
       */
      if (pCounterBuffers &&
          cb_idx < counterBufferCount &&
          pCounterBuffers[cb_idx] != VK_NULL_HANDLE) {
         ANV_FROM_HANDLE(anv_buffer, counter_buffer, pCounterBuffers[cb_idx]);
         uint64_t offset = pCounterBufferOffsets ?
                           pCounterBufferOffsets[cb_idx] : 0;

         anv_batch_emit(&cmd_buffer->batch, GENX(MI_STORE_REGISTER_MEM), srm) {
            srm.MemoryAddress    = anv_address_add(counter_buffer->address,
                                                   offset);
            srm.RegisterAddress  = GENX(SO_WRITE_OFFSET0_num) + idx * 4;
         }
      }
   }

   cmd_buffer->state.xfb_enabled = false;
   cmd_buffer->state.gfx.dirty |= ANV_CMD_DIRTY_XFB_ENABLE;
}

static void
genX(cmd_buffer_flush_compute_state)(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_cmd_compute_state *comp_state = &cmd_buffer->state.compute;
   struct anv_compute_pipeline *pipeline = comp_state->pipeline;

   assert(pipeline->cs);

   genX(cmd_buffer_config_l3)(cmd_buffer, pipeline->base.l3_config);

   genX(flush_pipeline_select_gpgpu)(cmd_buffer);

   /* Apply any pending pipeline flushes we may have.  We want to apply them
    * now because, if any of those flushes are for things like push constants,
    * the GPU will read the state at weird times.
    */
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   if (cmd_buffer->state.compute.pipeline_dirty) {
      /* From the Sky Lake PRM Vol 2a, MEDIA_VFE_STATE:
       *
       *    "A stalling PIPE_CONTROL is required before MEDIA_VFE_STATE unless
       *    the only bits that are changed are scoreboard related: Scoreboard
       *    Enable, Scoreboard Type, Scoreboard Mask, Scoreboard * Delta. For
       *    these scoreboard related states, a MEDIA_STATE_FLUSH is
       *    sufficient."
       */
      anv_add_pending_pipe_bits(cmd_buffer,
                              ANV_PIPE_CS_STALL_BIT,
                              "flush compute state");
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      anv_batch_emit_batch(&cmd_buffer->batch, &pipeline->base.batch);

      /* The workgroup size of the pipeline affects our push constant layout
       * so flag push constants as dirty if we change the pipeline.
       */
      cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_COMPUTE_BIT;
   }

   if ((cmd_buffer->state.descriptors_dirty & VK_SHADER_STAGE_COMPUTE_BIT) ||
       cmd_buffer->state.compute.pipeline_dirty) {
      flush_descriptor_sets(cmd_buffer,
                            &cmd_buffer->state.compute.base,
                            VK_SHADER_STAGE_COMPUTE_BIT,
                            &pipeline->cs, 1);
      cmd_buffer->state.descriptors_dirty &= ~VK_SHADER_STAGE_COMPUTE_BIT;

      uint32_t iface_desc_data_dw[GENX(INTERFACE_DESCRIPTOR_DATA_length)];
      struct GENX(INTERFACE_DESCRIPTOR_DATA) desc = {
         .BindingTablePointer =
            cmd_buffer->state.binding_tables[MESA_SHADER_COMPUTE].offset,
         .SamplerStatePointer =
            cmd_buffer->state.samplers[MESA_SHADER_COMPUTE].offset,
      };
      GENX(INTERFACE_DESCRIPTOR_DATA_pack)(NULL, iface_desc_data_dw, &desc);

      struct anv_state state =
         anv_cmd_buffer_merge_dynamic(cmd_buffer, iface_desc_data_dw,
                                      pipeline->interface_descriptor_data,
                                      GENX(INTERFACE_DESCRIPTOR_DATA_length),
                                      64);

      uint32_t size = GENX(INTERFACE_DESCRIPTOR_DATA_length) * sizeof(uint32_t);
      anv_batch_emit(&cmd_buffer->batch,
                     GENX(MEDIA_INTERFACE_DESCRIPTOR_LOAD), mid) {
         mid.InterfaceDescriptorTotalLength        = size;
         mid.InterfaceDescriptorDataStartAddress   = state.offset;
      }
   }

   if (cmd_buffer->state.push_constants_dirty & VK_SHADER_STAGE_COMPUTE_BIT) {
      comp_state->push_data =
         anv_cmd_buffer_cs_push_constants(cmd_buffer);

      if (comp_state->push_data.alloc_size) {
         anv_batch_emit(&cmd_buffer->batch, GENX(MEDIA_CURBE_LOAD), curbe) {
            curbe.CURBETotalDataLength    = comp_state->push_data.alloc_size;
            curbe.CURBEDataStartAddress   = comp_state->push_data.offset;
         }
      }

      cmd_buffer->state.push_constants_dirty &= ~VK_SHADER_STAGE_COMPUTE_BIT;
   }

   cmd_buffer->state.compute.pipeline_dirty = false;

   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
}

#if GFX_VER == 7

static VkResult
verify_cmd_parser(const struct anv_device *device,
                  int required_version,
                  const char *function)
{
   if (device->physical->cmd_parser_version < required_version) {
      return vk_errorf(device->physical, VK_ERROR_FEATURE_NOT_PRESENT,
                       "cmd parser version %d is required for %s",
                       required_version, function);
   } else {
      return VK_SUCCESS;
   }
}

#endif

static void
anv_cmd_buffer_push_base_group_id(struct anv_cmd_buffer *cmd_buffer,
                                  uint32_t baseGroupX,
                                  uint32_t baseGroupY,
                                  uint32_t baseGroupZ)
{
   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   struct anv_push_constants *push =
      &cmd_buffer->state.compute.base.push_constants;
   if (push->cs.base_work_group_id[0] != baseGroupX ||
       push->cs.base_work_group_id[1] != baseGroupY ||
       push->cs.base_work_group_id[2] != baseGroupZ) {
      push->cs.base_work_group_id[0] = baseGroupX;
      push->cs.base_work_group_id[1] = baseGroupY;
      push->cs.base_work_group_id[2] = baseGroupZ;

      cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_COMPUTE_BIT;
   }
}

static inline void
emit_gpgpu_walker(struct anv_cmd_buffer *cmd_buffer,
                  const struct anv_compute_pipeline *pipeline, bool indirect,
                  const struct brw_cs_prog_data *prog_data,
                  uint32_t groupCountX, uint32_t groupCountY,
                  uint32_t groupCountZ)
{
   bool predicate = (GFX_VER <= 7 && indirect) ||
      cmd_buffer->state.conditional_render_enabled;

   const struct intel_device_info *devinfo = pipeline->base.device->info;
   const struct brw_cs_dispatch_info dispatch =
      brw_cs_get_dispatch_info(devinfo, prog_data, NULL);

   anv_batch_emit(&cmd_buffer->batch, GENX(GPGPU_WALKER), ggw) {
      ggw.IndirectParameterEnable      = indirect;
      ggw.PredicateEnable              = predicate;
      ggw.SIMDSize                     = dispatch.simd_size / 16;
      ggw.ThreadDepthCounterMaximum    = 0;
      ggw.ThreadHeightCounterMaximum   = 0;
      ggw.ThreadWidthCounterMaximum    = dispatch.threads - 1;
      ggw.ThreadGroupIDXDimension      = groupCountX;
      ggw.ThreadGroupIDYDimension      = groupCountY;
      ggw.ThreadGroupIDZDimension      = groupCountZ;
      ggw.RightExecutionMask           = dispatch.right_mask;
      ggw.BottomExecutionMask          = 0xffffffff;
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MEDIA_STATE_FLUSH), msf);
}

static inline void
emit_cs_walker(struct anv_cmd_buffer *cmd_buffer,
               const struct anv_compute_pipeline *pipeline, bool indirect,
               const struct brw_cs_prog_data *prog_data,
               uint32_t groupCountX, uint32_t groupCountY,
               uint32_t groupCountZ)
{
   emit_gpgpu_walker(cmd_buffer, pipeline, indirect, prog_data, groupCountX,
                     groupCountY, groupCountZ);
}

void genX(CmdDispatchBase)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_compute_pipeline *pipeline = cmd_buffer->state.compute.pipeline;
   const struct brw_cs_prog_data *prog_data = get_cs_prog_data(pipeline);

   anv_cmd_buffer_push_base_group_id(cmd_buffer, baseGroupX,
                                     baseGroupY, baseGroupZ);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_COMPUTE,
                        "compute",
                        groupCountX * groupCountY * groupCountZ *
                        prog_data->local_size[0] * prog_data->local_size[1] *
                        prog_data->local_size[2]);

   trace_intel_begin_compute(&cmd_buffer->trace);

   if (prog_data->uses_num_work_groups) {
      struct anv_state state =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, 12, 4);
      uint32_t *sizes = state.map;
      sizes[0] = groupCountX;
      sizes[1] = groupCountY;
      sizes[2] = groupCountZ;
      cmd_buffer->state.compute.num_workgroups = (struct anv_address) {
         .bo = cmd_buffer->device->dynamic_state_pool.block_pool.bo,
         .offset = state.offset,
      };

      /* The num_workgroups buffer goes in the binding table */
      cmd_buffer->state.descriptors_dirty |= VK_SHADER_STAGE_COMPUTE_BIT;
   }

   genX(cmd_buffer_flush_compute_state)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   emit_cs_walker(cmd_buffer, pipeline, false, prog_data, groupCountX,
                  groupCountY, groupCountZ);

   trace_intel_end_compute(&cmd_buffer->trace,
                           groupCountX, groupCountY, groupCountZ);
}

#define GPGPU_DISPATCHDIMX 0x2500
#define GPGPU_DISPATCHDIMY 0x2504
#define GPGPU_DISPATCHDIMZ 0x2508

void genX(CmdDispatchIndirect)(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    _buffer,
    VkDeviceSize                                offset)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, buffer, _buffer);
   struct anv_compute_pipeline *pipeline = cmd_buffer->state.compute.pipeline;
   const struct brw_cs_prog_data *prog_data = get_cs_prog_data(pipeline);
   struct anv_address addr = anv_address_add(buffer->address, offset);
   UNUSED struct anv_batch *batch = &cmd_buffer->batch;

   anv_cmd_buffer_push_base_group_id(cmd_buffer, 0, 0, 0);

#if GFX_VER == 7
   /* Linux 4.4 added command parser version 5 which allows the GPGPU
    * indirect dispatch registers to be written.
    */
   if (verify_cmd_parser(cmd_buffer->device, 5,
                         "vkCmdDispatchIndirect") != VK_SUCCESS)
      return;
#endif

   anv_measure_snapshot(cmd_buffer,
                        INTEL_SNAPSHOT_COMPUTE,
                        "compute indirect",
                        0);
   trace_intel_begin_compute(&cmd_buffer->trace);

   if (prog_data->uses_num_work_groups) {
      cmd_buffer->state.compute.num_workgroups = addr;

      /* The num_workgroups buffer goes in the binding table */
      cmd_buffer->state.descriptors_dirty |= VK_SHADER_STAGE_COMPUTE_BIT;
   }

   genX(cmd_buffer_flush_compute_state)(cmd_buffer);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   struct mi_value size_x = mi_mem32(anv_address_add(addr, 0));
   struct mi_value size_y = mi_mem32(anv_address_add(addr, 4));
   struct mi_value size_z = mi_mem32(anv_address_add(addr, 8));

   mi_store(&b, mi_reg32(GPGPU_DISPATCHDIMX), size_x);
   mi_store(&b, mi_reg32(GPGPU_DISPATCHDIMY), size_y);
   mi_store(&b, mi_reg32(GPGPU_DISPATCHDIMZ), size_z);

#if GFX_VER <= 7
   /* predicate = (compute_dispatch_indirect_x_size == 0); */
   mi_store(&b, mi_reg64(MI_PREDICATE_SRC0), size_x);
   mi_store(&b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(0));
   anv_batch_emit(batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOAD;
      mip.CombineOperation = COMBINE_SET;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }

   /* predicate |= (compute_dispatch_indirect_y_size == 0); */
   mi_store(&b, mi_reg32(MI_PREDICATE_SRC0), size_y);
   anv_batch_emit(batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOAD;
      mip.CombineOperation = COMBINE_OR;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }

   /* predicate |= (compute_dispatch_indirect_z_size == 0); */
   mi_store(&b, mi_reg32(MI_PREDICATE_SRC0), size_z);
   anv_batch_emit(batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOAD;
      mip.CombineOperation = COMBINE_OR;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }

   /* predicate = !predicate; */
   anv_batch_emit(batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOADINV;
      mip.CombineOperation = COMBINE_OR;
      mip.CompareOperation = COMPARE_FALSE;
   }

#if GFX_VERx10 == 75
   if (cmd_buffer->state.conditional_render_enabled) {
      /* predicate &= !(conditional_rendering_predicate == 0); */
      mi_store(&b, mi_reg32(MI_PREDICATE_SRC0),
                   mi_reg32(ANV_PREDICATE_RESULT_REG));
      anv_batch_emit(batch, GENX(MI_PREDICATE), mip) {
         mip.LoadOperation    = LOAD_LOADINV;
         mip.CombineOperation = COMBINE_AND;
         mip.CompareOperation = COMPARE_SRCS_EQUAL;
      }
   }
#endif

#else /* GFX_VER > 7 */
   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);
#endif

   emit_cs_walker(cmd_buffer, pipeline, true, prog_data, 0, 0, 0);

   trace_intel_end_compute(&cmd_buffer->trace, 0, 0, 0);
}

static void
genX(flush_pipeline_select)(struct anv_cmd_buffer *cmd_buffer,
                            uint32_t pipeline)
{
   UNUSED const struct intel_device_info *devinfo = cmd_buffer->device->info;

   if (cmd_buffer->state.current_pipeline == pipeline)
      return;

#if GFX_VER >= 8
   /* From the Broadwell PRM, Volume 2a: Instructions, PIPELINE_SELECT:
    *
    *   Software must clear the COLOR_CALC_STATE Valid field in
    *   3DSTATE_CC_STATE_POINTERS command prior to send a PIPELINE_SELECT
    *   with Pipeline Select set to GPGPU.
    *
    * The internal hardware docs recommend the same workaround for Gfx9
    * hardware too.
    */
   if (pipeline == GPGPU)
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_CC_STATE_POINTERS), t);
#endif

   /* From "BXML » GT » MI » vol1a GPU Overview » [Instruction]
    * PIPELINE_SELECT [DevBWR+]":
    *
    *   Project: DEVSNB+
    *
    *   Software must ensure all the write caches are flushed through a
    *   stalling PIPE_CONTROL command followed by another PIPE_CONTROL
    *   command to invalidate read only caches prior to programming
    *   MI_PIPELINE_SELECT command to change the Pipeline Select Mode.
    *
    * Note the cmd_buffer_apply_pipe_flushes will split this into two
    * PIPE_CONTROLs.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             ANV_PIPE_DEPTH_CACHE_FLUSH_BIT |
                             ANV_PIPE_HDC_PIPELINE_FLUSH_BIT |
                             ANV_PIPE_CS_STALL_BIT |
                             ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT |
                             ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT |
                             ANV_PIPE_STATE_CACHE_INVALIDATE_BIT |
                             ANV_PIPE_INSTRUCTION_CACHE_INVALIDATE_BIT |
                             ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT,
                             "flush and invalidate for PIPELINE_SELECT");
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   anv_batch_emit(&cmd_buffer->batch, GENX(PIPELINE_SELECT), ps) {
      ps.PipelineSelection = pipeline;
   }

   cmd_buffer->state.current_pipeline = pipeline;
}

void
genX(flush_pipeline_select_3d)(struct anv_cmd_buffer *cmd_buffer)
{
   genX(flush_pipeline_select)(cmd_buffer, _3D);
}

void
genX(flush_pipeline_select_gpgpu)(struct anv_cmd_buffer *cmd_buffer)
{
   genX(flush_pipeline_select)(cmd_buffer, GPGPU);
}

void
genX(cmd_buffer_emit_gfx7_depth_flush)(struct anv_cmd_buffer *cmd_buffer)
{
   if (GFX_VER >= 8)
      return;

   /* From the Haswell PRM, documentation for 3DSTATE_DEPTH_BUFFER:
    *
    *    "Restriction: Prior to changing Depth/Stencil Buffer state (i.e., any
    *    combination of 3DSTATE_DEPTH_BUFFER, 3DSTATE_CLEAR_PARAMS,
    *    3DSTATE_STENCIL_BUFFER, 3DSTATE_HIER_DEPTH_BUFFER) SW must first
    *    issue a pipelined depth stall (PIPE_CONTROL with Depth Stall bit
    *    set), followed by a pipelined depth cache flush (PIPE_CONTROL with
    *    Depth Flush Bit set, followed by another pipelined depth stall
    *    (PIPE_CONTROL with Depth Stall Bit set), unless SW can otherwise
    *    guarantee that the pipeline from WM onwards is already flushed (e.g.,
    *    via a preceding MI_FLUSH)."
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pipe) {
      pipe.DepthStallEnable = true;
      anv_debug_dump_pc(pipe);
   }
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pipe) {
      pipe.DepthCacheFlushEnable = true;
      anv_debug_dump_pc(pipe);
   }
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pipe) {
      pipe.DepthStallEnable = true;
      anv_debug_dump_pc(pipe);
   }
}

/* From the Skylake PRM, 3DSTATE_VERTEX_BUFFERS:
 *
 *    "The VF cache needs to be invalidated before binding and then using
 *    Vertex Buffers that overlap with any previously bound Vertex Buffer
 *    (at a 64B granularity) since the last invalidation.  A VF cache
 *    invalidate is performed by setting the "VF Cache Invalidation Enable"
 *    bit in PIPE_CONTROL."
 *
 * This is implemented by carefully tracking all vertex and index buffer
 * bindings and flushing if the cache ever ends up with a range in the cache
 * that would exceed 4 GiB.  This is implemented in three parts:
 *
 *    1. genX(cmd_buffer_set_binding_for_gfx8_vb_flush)() which must be called
 *       every time a 3DSTATE_VERTEX_BUFFER packet is emitted and informs the
 *       tracking code of the new binding.  If this new binding would cause
 *       the cache to have a too-large range on the next draw call, a pipeline
 *       stall and VF cache invalidate are added to pending_pipeline_bits.
 *
 *    2. genX(cmd_buffer_apply_pipe_flushes)() resets the cache tracking to
 *       empty whenever we emit a VF invalidate.
 *
 *    3. genX(cmd_buffer_update_dirty_vbs_for_gfx8_vb_flush)() must be called
 *       after every 3DPRIMITIVE and copies the bound range into the dirty
 *       range for each used buffer.  This has to be a separate step because
 *       we don't always re-bind all buffers and so 1. can't know which
 *       buffers are actually bound.
 */
void
genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(struct anv_cmd_buffer *cmd_buffer,
                                               int vb_index,
                                               struct anv_address vb_address,
                                               uint32_t vb_size)
{
   if (GFX_VER < 8 || anv_use_relocations(cmd_buffer->device->physical))
      return;

   struct anv_vb_cache_range *bound, *dirty;
   if (vb_index == -1) {
      bound = &cmd_buffer->state.gfx.ib_bound_range;
      dirty = &cmd_buffer->state.gfx.ib_dirty_range;
   } else {
      assert(vb_index >= 0);
      assert(vb_index < ARRAY_SIZE(cmd_buffer->state.gfx.vb_bound_ranges));
      assert(vb_index < ARRAY_SIZE(cmd_buffer->state.gfx.vb_dirty_ranges));
      bound = &cmd_buffer->state.gfx.vb_bound_ranges[vb_index];
      dirty = &cmd_buffer->state.gfx.vb_dirty_ranges[vb_index];
   }

   if (anv_gfx8_9_vb_cache_range_needs_workaround(bound, dirty,
                                                  vb_address,
                                                  vb_size)) {
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_CS_STALL_BIT |
                                ANV_PIPE_VF_CACHE_INVALIDATE_BIT,
                                "vb > 32b range");
   }
}

void
genX(cmd_buffer_update_dirty_vbs_for_gfx8_vb_flush)(struct anv_cmd_buffer *cmd_buffer,
                                                    uint32_t access_type,
                                                    uint64_t vb_used)
{
   if (GFX_VER < 8 || anv_use_relocations(cmd_buffer->device->physical))
      return;

   if (access_type == RANDOM) {
      /* We have an index buffer */
      struct anv_vb_cache_range *bound = &cmd_buffer->state.gfx.ib_bound_range;
      struct anv_vb_cache_range *dirty = &cmd_buffer->state.gfx.ib_dirty_range;

      anv_merge_vb_cache_range(dirty, bound);
   }

   uint64_t mask = vb_used;
   while (mask) {
      int i = u_bit_scan64(&mask);
      assert(i >= 0);
      assert(i < ARRAY_SIZE(cmd_buffer->state.gfx.vb_bound_ranges));
      assert(i < ARRAY_SIZE(cmd_buffer->state.gfx.vb_dirty_ranges));

      struct anv_vb_cache_range *bound, *dirty;
      bound = &cmd_buffer->state.gfx.vb_bound_ranges[i];
      dirty = &cmd_buffer->state.gfx.vb_dirty_ranges[i];

      anv_merge_vb_cache_range(dirty, bound);
   }
}

static void
cmd_buffer_emit_depth_stencil(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_device *device = cmd_buffer->device;
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;

   /* FIXME: Width and Height are wrong */

   genX(cmd_buffer_emit_gfx7_depth_flush)(cmd_buffer);

   uint32_t *dw = anv_batch_emit_dwords(&cmd_buffer->batch,
                                        device->isl_dev.ds.size / 4);
   if (dw == NULL)
      return;

   struct isl_view isl_view = {};
   struct isl_depth_stencil_hiz_emit_info info = {
      .view = &isl_view,
      .mocs = anv_mocs(device, NULL, ISL_SURF_USAGE_DEPTH_BIT),
   };

   if (gfx->depth_att.iview != NULL) {
      isl_view = gfx->depth_att.iview->planes[0].isl;
   } else if (gfx->stencil_att.iview != NULL) {
      isl_view = gfx->stencil_att.iview->planes[0].isl;
   }

   if (gfx->view_mask) {
      assert(isl_view.array_len == 0 ||
             isl_view.array_len >= util_last_bit(gfx->view_mask));
      isl_view.array_len = util_last_bit(gfx->view_mask);
   } else {
      assert(isl_view.array_len == 0 ||
             isl_view.array_len >= util_last_bit(gfx->layer_count));
      isl_view.array_len = gfx->layer_count;
   }

   if (gfx->depth_att.iview != NULL) {
      const struct anv_image_view *iview = gfx->depth_att.iview;
      const struct anv_image *image = iview->image;

      const uint32_t depth_plane =
         anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_DEPTH_BIT);
      const struct anv_surface *depth_surface =
         &image->planes[depth_plane].primary_surface;
      const struct anv_address depth_address =
         anv_image_address(image, &depth_surface->memory_range);

      info.depth_surf = &depth_surface->isl;

      info.depth_address =
         anv_batch_emit_reloc(&cmd_buffer->batch,
                              dw + device->isl_dev.ds.depth_offset / 4,
                              depth_address.bo, depth_address.offset);
      info.mocs =
         anv_mocs(device, depth_address.bo, ISL_SURF_USAGE_DEPTH_BIT);

      info.hiz_usage = gfx->depth_att.aux_usage;
      if (info.hiz_usage != ISL_AUX_USAGE_NONE) {
         assert(isl_aux_usage_has_hiz(info.hiz_usage));

         const struct anv_surface *hiz_surface =
            &image->planes[depth_plane].aux_surface;
         const struct anv_address hiz_address =
            anv_image_address(image, &hiz_surface->memory_range);

         info.hiz_surf = &hiz_surface->isl;

         info.hiz_address =
            anv_batch_emit_reloc(&cmd_buffer->batch,
                                 dw + device->isl_dev.ds.hiz_offset / 4,
                                 hiz_address.bo, hiz_address.offset);

         info.depth_clear_value = ANV_HZ_FC_VAL;
      }
   }

   if (gfx->stencil_att.iview != NULL) {
      const struct anv_image_view *iview = gfx->stencil_att.iview;
      const struct anv_image *image = iview->image;

      const uint32_t stencil_plane =
         anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_STENCIL_BIT);
      const struct anv_surface *stencil_surface =
         &image->planes[stencil_plane].primary_surface;
      const struct anv_address stencil_address =
         anv_image_address(image, &stencil_surface->memory_range);

      info.stencil_surf = &stencil_surface->isl;

      info.stencil_aux_usage = image->planes[stencil_plane].aux_usage;
      info.stencil_address =
         anv_batch_emit_reloc(&cmd_buffer->batch,
                              dw + device->isl_dev.ds.stencil_offset / 4,
                              stencil_address.bo, stencil_address.offset);
      info.mocs =
         anv_mocs(device, stencil_address.bo, ISL_SURF_USAGE_STENCIL_BIT);
   }

   isl_emit_depth_stencil_hiz_s(&device->isl_dev, dw, &info);

   cmd_buffer->state.hiz_enabled = isl_aux_usage_has_hiz(info.hiz_usage);
}

static VkImageLayout
attachment_initial_layout(const VkRenderingAttachmentInfo *att)
{
   const VkRenderingAttachmentInitialLayoutInfoMESA *layout_info =
      vk_find_struct_const(att->pNext,
                           RENDERING_ATTACHMENT_INITIAL_LAYOUT_INFO_MESA);
   if (layout_info != NULL)
      return layout_info->initialLayout;

   return att->imageLayout;
}

void genX(CmdBeginRendering)(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   VkResult result;

   if (!is_render_queue_cmd_buffer(cmd_buffer)) {
      assert(!"Trying to start a render pass on non-render queue!");
      anv_batch_set_error(&cmd_buffer->batch, VK_ERROR_UNKNOWN);
      return;
   }

   anv_measure_beginrenderpass(cmd_buffer);
   trace_intel_begin_render_pass(&cmd_buffer->trace);

   gfx->rendering_flags = pRenderingInfo->flags;
   gfx->render_area = pRenderingInfo->renderArea;
   gfx->view_mask = pRenderingInfo->viewMask;
   gfx->layer_count = pRenderingInfo->layerCount;
   gfx->samples = 0;

   const bool is_multiview = gfx->view_mask != 0;
   const VkRect2D render_area = gfx->render_area;
   const uint32_t layers =
      is_multiview ? util_last_bit(gfx->view_mask) : gfx->layer_count;

   /* The framebuffer size is at least large enough to contain the render
    * area.  Because a zero renderArea is possible, we MAX with 1.
    */
   struct isl_extent3d fb_size = {
      .w = MAX2(1, render_area.offset.x + render_area.extent.width),
      .h = MAX2(1, render_area.offset.y + render_area.extent.height),
      .d = layers,
   };

   const uint32_t color_att_count = pRenderingInfo->colorAttachmentCount;
   result = anv_cmd_buffer_init_attachments(cmd_buffer, color_att_count);
   if (result != VK_SUCCESS)
      return;

   genX(flush_pipeline_select_3d)(cmd_buffer);

   for (uint32_t i = 0; i < gfx->color_att_count; i++) {
      if (pRenderingInfo->pColorAttachments[i].imageView == VK_NULL_HANDLE)
         continue;

      const VkRenderingAttachmentInfo *att =
         &pRenderingInfo->pColorAttachments[i];
      ANV_FROM_HANDLE(anv_image_view, iview, att->imageView);
      const VkImageLayout initial_layout = attachment_initial_layout(att);

      assert(render_area.offset.x + render_area.extent.width <=
             iview->vk.extent.width);
      assert(render_area.offset.y + render_area.extent.height <=
             iview->vk.extent.height);
      assert(layers <= iview->vk.layer_count);

      fb_size.w = MAX2(fb_size.w, iview->vk.extent.width);
      fb_size.h = MAX2(fb_size.h, iview->vk.extent.height);

      assert(gfx->samples == 0 || gfx->samples == iview->vk.image->samples);
      gfx->samples |= iview->vk.image->samples;

      enum isl_aux_usage aux_usage =
         anv_layout_to_aux_usage(cmd_buffer->device->info,
                                 iview->image,
                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 att->imageLayout);

      union isl_color_value fast_clear_color = { .u32 = { 0, } };

      if (att->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR &&
          !(gfx->rendering_flags & VK_RENDERING_RESUMING_BIT)) {
         const union isl_color_value clear_color =
            vk_to_isl_color_with_format(att->clearValue.color,
                                        iview->planes[0].isl.format);

         /* We only support fast-clears on the first layer */
         const bool fast_clear =
            (!is_multiview || (gfx->view_mask & 1)) &&
            anv_can_fast_clear_color_view(cmd_buffer->device, iview,
                                          att->imageLayout, clear_color,
                                          layers, render_area);

         if (att->imageLayout != initial_layout) {
            assert(render_area.offset.x == 0 && render_area.offset.y == 0 &&
                   render_area.extent.width == iview->vk.extent.width &&
                   render_area.extent.height == iview->vk.extent.height);
            if (is_multiview) {
               u_foreach_bit(view, gfx->view_mask) {
                  transition_color_buffer(cmd_buffer, iview->image,
                                          VK_IMAGE_ASPECT_COLOR_BIT,
                                          iview->vk.base_mip_level, 1,
                                          iview->vk.base_array_layer + view,
                                          1, /* layer_count */
                                          initial_layout, att->imageLayout,
                                          VK_QUEUE_FAMILY_IGNORED,
                                          VK_QUEUE_FAMILY_IGNORED,
                                          fast_clear);
               }
            } else {
               transition_color_buffer(cmd_buffer, iview->image,
                                       VK_IMAGE_ASPECT_COLOR_BIT,
                                       iview->vk.base_mip_level, 1,
                                       iview->vk.base_array_layer,
                                       gfx->layer_count,
                                       initial_layout, att->imageLayout,
                                       VK_QUEUE_FAMILY_IGNORED,
                                       VK_QUEUE_FAMILY_IGNORED,
                                       fast_clear);
            }
         }

         uint32_t clear_view_mask = pRenderingInfo->viewMask;
         uint32_t base_clear_layer = iview->vk.base_array_layer;
         uint32_t clear_layer_count = gfx->layer_count;
         if (fast_clear) {
            /* We only support fast-clears on the first layer */
            assert(iview->vk.base_mip_level == 0 &&
                   iview->vk.base_array_layer == 0);

            fast_clear_color = clear_color;

            if (iview->image->vk.samples == 1) {
               anv_image_ccs_op(cmd_buffer, iview->image,
                                iview->planes[0].isl.format,
                                iview->planes[0].isl.swizzle,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                0, 0, 1, ISL_AUX_OP_FAST_CLEAR,
                                &fast_clear_color,
                                false);
            } else {
               anv_image_mcs_op(cmd_buffer, iview->image,
                                iview->planes[0].isl.format,
                                iview->planes[0].isl.swizzle,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                0, 1, ISL_AUX_OP_FAST_CLEAR,
                                &fast_clear_color,
                                false);
            }
            clear_view_mask &= ~1u;
            base_clear_layer++;
            clear_layer_count--;

            if (isl_color_value_is_zero(clear_color,
                                        iview->planes[0].isl.format)) {
               /* This image has the auxiliary buffer enabled. We can mark the
                * subresource as not needing a resolve because the clear color
                * will match what's in every RENDER_SURFACE_STATE object when
                * it's being used for sampling.
                */
               set_image_fast_clear_state(cmd_buffer, iview->image,
                                          VK_IMAGE_ASPECT_COLOR_BIT,
                                          ANV_FAST_CLEAR_DEFAULT_VALUE);
            } else {
               set_image_fast_clear_state(cmd_buffer, iview->image,
                                          VK_IMAGE_ASPECT_COLOR_BIT,
                                          ANV_FAST_CLEAR_ANY);
            }
         }

         if (is_multiview) {
            u_foreach_bit(view, clear_view_mask) {
               anv_image_clear_color(cmd_buffer, iview->image,
                                     VK_IMAGE_ASPECT_COLOR_BIT,
                                     aux_usage,
                                     iview->planes[0].isl.format,
                                     iview->planes[0].isl.swizzle,
                                     iview->vk.base_mip_level,
                                     iview->vk.base_array_layer + view, 1,
                                     render_area, clear_color);
            }
         } else {
            anv_image_clear_color(cmd_buffer, iview->image,
                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                  aux_usage,
                                  iview->planes[0].isl.format,
                                  iview->planes[0].isl.swizzle,
                                  iview->vk.base_mip_level,
                                  base_clear_layer, clear_layer_count,
                                  render_area, clear_color);
         }
      } else {
         /* If not LOAD_OP_CLEAR, we shouldn't have a layout transition. */
         assert(att->imageLayout == initial_layout);
      }

      gfx->color_att[i].vk_format = iview->vk.format;
      gfx->color_att[i].iview = iview;
      gfx->color_att[i].layout = att->imageLayout;
      gfx->color_att[i].aux_usage = aux_usage;

      struct isl_view isl_view = iview->planes[0].isl;
      if (pRenderingInfo->viewMask) {
         assert(isl_view.array_len >= util_last_bit(pRenderingInfo->viewMask));
         isl_view.array_len = util_last_bit(pRenderingInfo->viewMask);
      } else {
         assert(isl_view.array_len >= pRenderingInfo->layerCount);
         isl_view.array_len = pRenderingInfo->layerCount;
      }

      anv_image_fill_surface_state(cmd_buffer->device,
                                   iview->image,
                                   VK_IMAGE_ASPECT_COLOR_BIT,
                                   &isl_view,
                                   ISL_SURF_USAGE_RENDER_TARGET_BIT,
                                   aux_usage, &fast_clear_color,
                                   0, /* anv_image_view_state_flags */
                                   &gfx->color_att[i].surface_state,
                                   NULL);

      add_surface_state_relocs(cmd_buffer, gfx->color_att[i].surface_state);

      if ((att->loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ||
           (gfx->rendering_flags & VK_RENDERING_RESUMING_BIT)) &&
          iview->image->planes[0].aux_usage != ISL_AUX_USAGE_NONE &&
          iview->planes[0].isl.base_level == 0 &&
          iview->planes[0].isl.base_array_layer == 0) {
         genX(copy_fast_clear_dwords)(cmd_buffer,
                                      gfx->color_att[i].surface_state.state,
                                      iview->image,
                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                      false /* copy to ss */);
      }

      if (att->resolveMode != VK_RESOLVE_MODE_NONE) {
         gfx->color_att[i].resolve_mode = att->resolveMode;
         gfx->color_att[i].resolve_iview =
            anv_image_view_from_handle(att->resolveImageView);
         gfx->color_att[i].resolve_layout = att->resolveImageLayout;
      }
   }

   anv_cmd_graphic_state_update_has_uint_rt(gfx);

   const struct anv_image_view *ds_iview = NULL;
   const VkRenderingAttachmentInfo *d_att = pRenderingInfo->pDepthAttachment;
   const VkRenderingAttachmentInfo *s_att = pRenderingInfo->pStencilAttachment;
   if ((d_att != NULL && d_att->imageView != VK_NULL_HANDLE) ||
       (s_att != NULL && s_att->imageView != VK_NULL_HANDLE)) {
      const struct anv_image_view *d_iview = NULL, *s_iview = NULL;
      VkImageLayout depth_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout stencil_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout initial_depth_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout initial_stencil_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      enum isl_aux_usage depth_aux_usage = ISL_AUX_USAGE_NONE;
      enum isl_aux_usage stencil_aux_usage = ISL_AUX_USAGE_NONE;
      float depth_clear_value = 0;
      uint32_t stencil_clear_value = 0;

      if (d_att != NULL && d_att->imageView != VK_NULL_HANDLE) {
         d_iview = anv_image_view_from_handle(d_att->imageView);
         initial_depth_layout = attachment_initial_layout(d_att);
         depth_layout = d_att->imageLayout;
         depth_aux_usage =
            anv_layout_to_aux_usage(cmd_buffer->device->info,
                                    d_iview->image,
                                    VK_IMAGE_ASPECT_DEPTH_BIT,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                    depth_layout);
         depth_clear_value = d_att->clearValue.depthStencil.depth;
      }

      if (s_att != NULL && s_att->imageView != VK_NULL_HANDLE) {
         s_iview = anv_image_view_from_handle(s_att->imageView);
         initial_stencil_layout = attachment_initial_layout(s_att);
         stencil_layout = s_att->imageLayout;
         stencil_aux_usage =
            anv_layout_to_aux_usage(cmd_buffer->device->info,
                                    s_iview->image,
                                    VK_IMAGE_ASPECT_STENCIL_BIT,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                    stencil_layout);
         stencil_clear_value = s_att->clearValue.depthStencil.stencil;
      }

      assert(s_iview == NULL || d_iview == NULL || s_iview == d_iview);
      ds_iview = d_iview != NULL ? d_iview : s_iview;
      assert(ds_iview != NULL);

      assert(render_area.offset.x + render_area.extent.width <=
             ds_iview->vk.extent.width);
      assert(render_area.offset.y + render_area.extent.height <=
             ds_iview->vk.extent.height);
      assert(layers <= ds_iview->vk.layer_count);

      fb_size.w = MAX2(fb_size.w, ds_iview->vk.extent.width);
      fb_size.h = MAX2(fb_size.h, ds_iview->vk.extent.height);

      assert(gfx->samples == 0 || gfx->samples == ds_iview->vk.image->samples);
      gfx->samples |= ds_iview->vk.image->samples;

      VkImageAspectFlags clear_aspects = 0;
      if (d_iview != NULL && d_att->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR &&
          !(gfx->rendering_flags & VK_RENDERING_RESUMING_BIT))
         clear_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
      if (s_iview != NULL && s_att->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR &&
          !(gfx->rendering_flags & VK_RENDERING_RESUMING_BIT))
         clear_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;

      if (clear_aspects != 0) {
         const bool hiz_clear =
            anv_can_hiz_clear_ds_view(cmd_buffer->device, d_iview,
                                      depth_layout, clear_aspects,
                                      depth_clear_value,
                                      render_area);

         if (depth_layout != initial_depth_layout) {
            assert(render_area.offset.x == 0 && render_area.offset.y == 0 &&
                   render_area.extent.width == d_iview->vk.extent.width &&
                   render_area.extent.height == d_iview->vk.extent.height);

            if (is_multiview) {
               u_foreach_bit(view, gfx->view_mask) {
                  transition_depth_buffer(cmd_buffer, d_iview->image,
                                          d_iview->vk.base_array_layer + view,
                                          1 /* layer_count */,
                                          initial_depth_layout, depth_layout,
                                          hiz_clear);
               }
            } else {
               transition_depth_buffer(cmd_buffer, d_iview->image,
                                       d_iview->vk.base_array_layer,
                                       gfx->layer_count,
                                       initial_depth_layout, depth_layout,
                                       hiz_clear);
            }
         }

         if (stencil_layout != initial_stencil_layout) {
            assert(render_area.offset.x == 0 && render_area.offset.y == 0 &&
                   render_area.extent.width == s_iview->vk.extent.width &&
                   render_area.extent.height == s_iview->vk.extent.height);

            if (is_multiview) {
               u_foreach_bit(view, gfx->view_mask) {
                  transition_stencil_buffer(cmd_buffer, s_iview->image,
                                            s_iview->vk.base_mip_level, 1,
                                            s_iview->vk.base_array_layer + view,
                                            1 /* layer_count */,
                                            initial_stencil_layout,
                                            stencil_layout,
                                            hiz_clear);
               }
            } else {
               transition_stencil_buffer(cmd_buffer, s_iview->image,
                                         s_iview->vk.base_mip_level, 1,
                                         s_iview->vk.base_array_layer,
                                         gfx->layer_count,
                                         initial_stencil_layout,
                                         stencil_layout,
                                         hiz_clear);
            }
         }

         if (is_multiview) {
            uint32_t clear_view_mask = pRenderingInfo->viewMask;
            while (clear_view_mask) {
               int view = u_bit_scan(&clear_view_mask);

               uint32_t level = ds_iview->vk.base_mip_level;
               uint32_t layer = ds_iview->vk.base_array_layer + view;

               if (hiz_clear) {
                  anv_image_hiz_clear(cmd_buffer, ds_iview->image,
                                      clear_aspects,
                                      level, layer, 1,
                                      render_area,
                                      stencil_clear_value);
               } else {
                  anv_image_clear_depth_stencil(cmd_buffer, ds_iview->image,
                                                clear_aspects,
                                                depth_aux_usage,
                                                level, layer, 1,
                                                render_area,
                                                depth_clear_value,
                                                stencil_clear_value);
               }
            }
         } else {
            uint32_t level = ds_iview->vk.base_mip_level;
            uint32_t base_layer = ds_iview->vk.base_array_layer;
            uint32_t layer_count = gfx->layer_count;

            if (hiz_clear) {
               anv_image_hiz_clear(cmd_buffer, ds_iview->image,
                                   clear_aspects,
                                   level, base_layer, layer_count,
                                   render_area,
                                   stencil_clear_value);
            } else {
               anv_image_clear_depth_stencil(cmd_buffer, ds_iview->image,
                                             clear_aspects,
                                             depth_aux_usage,
                                             level, base_layer, layer_count,
                                             render_area,
                                             depth_clear_value,
                                             stencil_clear_value);
            }
         }
      } else {
         /* If not LOAD_OP_CLEAR, we shouldn't have a layout transition. */
         assert(depth_layout == initial_depth_layout);
         assert(stencil_layout == initial_stencil_layout);
      }

      if (d_iview != NULL) {
         gfx->depth_att.vk_format = d_iview->vk.format;
         gfx->depth_att.iview = d_iview;
         gfx->depth_att.layout = depth_layout;
         gfx->depth_att.aux_usage = depth_aux_usage;
         if (d_att != NULL && d_att->resolveMode != VK_RESOLVE_MODE_NONE) {
            assert(d_att->resolveImageView != VK_NULL_HANDLE);
            gfx->depth_att.resolve_mode = d_att->resolveMode;
            gfx->depth_att.resolve_iview =
               anv_image_view_from_handle(d_att->resolveImageView);
            gfx->depth_att.resolve_layout = d_att->resolveImageLayout;
         }
      }

      if (s_iview != NULL) {
         gfx->stencil_att.vk_format = s_iview->vk.format;
         gfx->stencil_att.iview = s_iview;
         gfx->stencil_att.layout = stencil_layout;
         gfx->stencil_att.aux_usage = stencil_aux_usage;
         if (s_att->resolveMode != VK_RESOLVE_MODE_NONE) {
            assert(s_att->resolveImageView != VK_NULL_HANDLE);
            gfx->stencil_att.resolve_mode = s_att->resolveMode;
            gfx->stencil_att.resolve_iview =
               anv_image_view_from_handle(s_att->resolveImageView);
            gfx->stencil_att.resolve_layout = s_att->resolveImageLayout;
         }
      }
   }

   /* Finally, now that we know the right size, set up the null surface */
   assert(util_bitcount(gfx->samples) <= 1);
   isl_null_fill_state(&cmd_buffer->device->isl_dev,
                       gfx->null_surface_state.map,
                       .size = fb_size);

   for (uint32_t i = 0; i < gfx->color_att_count; i++) {
      if (pRenderingInfo->pColorAttachments[i].imageView != VK_NULL_HANDLE)
         continue;

      isl_null_fill_state(&cmd_buffer->device->isl_dev,
                          gfx->color_att[i].surface_state.state.map,
                          .size = fb_size);
   }

   /****** We can now start emitting code to begin the render pass ******/

   gfx->dirty |= ANV_CMD_DIRTY_RENDER_TARGETS;

   /* Our implementation of VK_KHR_multiview uses instancing to draw the
    * different views.  If the client asks for instancing, we need to use the
    * Instance Data Step Rate to ensure that we repeat the client's
    * per-instance data once for each view.  Since this bit is in
    * VERTEX_BUFFER_STATE on gfx7, we need to dirty vertex buffers at the top
    * of each subpass.
    */
   if (GFX_VER == 7)
      gfx->vb_dirty |= ~0;

   /* It is possible to start a render pass with an old pipeline.  Because the
    * render pass and subpass index are both baked into the pipeline, this is
    * highly unlikely.  In order to do so, it requires that you have a render
    * pass with a single subpass and that you use that render pass twice
    * back-to-back and use the same pipeline at the start of the second render
    * pass as at the end of the first.  In order to avoid unpredictable issues
    * with this edge case, we just dirty the pipeline at the start of every
    * subpass.
    */
   gfx->dirty |= ANV_CMD_DIRTY_PIPELINE;

   cmd_buffer_emit_depth_stencil(cmd_buffer);
}

static void
cmd_buffer_mark_attachment_written(struct anv_cmd_buffer *cmd_buffer,
                                   struct anv_attachment *att,
                                   VkImageAspectFlagBits aspect)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const struct anv_image_view *iview = att->iview;

   if (iview == NULL)
      return;

   if (gfx->view_mask == 0) {
      genX(cmd_buffer_mark_image_written)(cmd_buffer, iview->image,
                                          aspect, att->aux_usage,
                                          iview->planes[0].isl.base_level,
                                          iview->planes[0].isl.base_array_layer,
                                          gfx->layer_count);
   } else {
      uint32_t res_view_mask = gfx->view_mask;
      while (res_view_mask) {
         int i = u_bit_scan(&res_view_mask);

         const uint32_t level = iview->planes[0].isl.base_level;
         const uint32_t layer = iview->planes[0].isl.base_array_layer + i;

         genX(cmd_buffer_mark_image_written)(cmd_buffer, iview->image,
                                             aspect, att->aux_usage,
                                             level, layer, 1);
      }
   }
}

static enum blorp_filter
vk_to_blorp_resolve_mode(VkResolveModeFlagBits vk_mode)
{
   switch (vk_mode) {
   case VK_RESOLVE_MODE_SAMPLE_ZERO_BIT:
      return BLORP_FILTER_SAMPLE_0;
   case VK_RESOLVE_MODE_AVERAGE_BIT:
      return BLORP_FILTER_AVERAGE;
   case VK_RESOLVE_MODE_MIN_BIT:
      return BLORP_FILTER_MIN_SAMPLE;
   case VK_RESOLVE_MODE_MAX_BIT:
      return BLORP_FILTER_MAX_SAMPLE;
   default:
      return BLORP_FILTER_NONE;
   }
}

static void
cmd_buffer_resolve_msaa_attachment(struct anv_cmd_buffer *cmd_buffer,
                                   const struct anv_attachment *att,
                                   VkImageLayout layout,
                                   VkImageAspectFlagBits aspect)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const struct anv_image_view *src_iview = att->iview;
   const struct anv_image_view *dst_iview = att->resolve_iview;

   enum isl_aux_usage src_aux_usage =
      anv_layout_to_aux_usage(cmd_buffer->device->info,
                              src_iview->image, aspect,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                              layout);

   enum isl_aux_usage dst_aux_usage =
      anv_layout_to_aux_usage(cmd_buffer->device->info,
                              dst_iview->image, aspect,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                              att->resolve_layout);

   enum blorp_filter filter = vk_to_blorp_resolve_mode(att->resolve_mode);

   const VkRect2D render_area = gfx->render_area;
   if (gfx->view_mask == 0) {
      anv_image_msaa_resolve(cmd_buffer,
                             src_iview->image, src_aux_usage,
                             src_iview->planes[0].isl.base_level,
                             src_iview->planes[0].isl.base_array_layer,
                             dst_iview->image, dst_aux_usage,
                             dst_iview->planes[0].isl.base_level,
                             dst_iview->planes[0].isl.base_array_layer,
                             aspect,
                             render_area.offset.x, render_area.offset.y,
                             render_area.offset.x, render_area.offset.y,
                             render_area.extent.width,
                             render_area.extent.height,
                             gfx->layer_count, filter);
   } else {
      uint32_t res_view_mask = gfx->view_mask;
      while (res_view_mask) {
         int i = u_bit_scan(&res_view_mask);

         anv_image_msaa_resolve(cmd_buffer,
                                src_iview->image, src_aux_usage,
                                src_iview->planes[0].isl.base_level,
                                src_iview->planes[0].isl.base_array_layer + i,
                                dst_iview->image, dst_aux_usage,
                                dst_iview->planes[0].isl.base_level,
                                dst_iview->planes[0].isl.base_array_layer + i,
                                aspect,
                                render_area.offset.x, render_area.offset.y,
                                render_area.offset.x, render_area.offset.y,
                                render_area.extent.width,
                                render_area.extent.height,
                                1, filter);
      }
   }
}

void genX(CmdEndRendering)(
    VkCommandBuffer                             commandBuffer)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   const bool is_multiview = gfx->view_mask != 0;
   const uint32_t layers =
      is_multiview ? util_last_bit(gfx->view_mask) : gfx->layer_count;

   bool has_color_resolve = false;
   for (uint32_t i = 0; i < gfx->color_att_count; i++) {
      cmd_buffer_mark_attachment_written(cmd_buffer, &gfx->color_att[i],
                                         VK_IMAGE_ASPECT_COLOR_BIT);

      /* Stash this off for later */
      if (gfx->color_att[i].resolve_mode != VK_RESOLVE_MODE_NONE &&
          !(gfx->rendering_flags & VK_RENDERING_SUSPENDING_BIT))
         has_color_resolve = true;
   }

   cmd_buffer_mark_attachment_written(cmd_buffer, &gfx->depth_att,
                                      VK_IMAGE_ASPECT_DEPTH_BIT);

   cmd_buffer_mark_attachment_written(cmd_buffer, &gfx->stencil_att,
                                      VK_IMAGE_ASPECT_STENCIL_BIT);

   if (has_color_resolve) {
      /* We are about to do some MSAA resolves.  We need to flush so that the
       * result of writes to the MSAA color attachments show up in the sampler
       * when we blit to the single-sampled resolve target.
       */
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT |
                                ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT,
                                "MSAA resolve");
   }

   if (gfx->depth_att.resolve_mode != VK_RESOLVE_MODE_NONE ||
       gfx->stencil_att.resolve_mode != VK_RESOLVE_MODE_NONE) {
      /* We are about to do some MSAA resolves.  We need to flush so that the
       * result of writes to the MSAA depth attachments show up in the sampler
       * when we blit to the single-sampled resolve target.
       */
      anv_add_pending_pipe_bits(cmd_buffer,
                              ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT |
                              ANV_PIPE_DEPTH_CACHE_FLUSH_BIT,
                              "MSAA resolve");
   }

   for (uint32_t i = 0; i < gfx->color_att_count; i++) {
      const struct anv_attachment *att = &gfx->color_att[i];
      if (att->resolve_mode == VK_RESOLVE_MODE_NONE ||
          (gfx->rendering_flags & VK_RENDERING_SUSPENDING_BIT))
         continue;

      cmd_buffer_resolve_msaa_attachment(cmd_buffer, att, att->layout,
                                         VK_IMAGE_ASPECT_COLOR_BIT);
   }

   if (gfx->depth_att.resolve_mode != VK_RESOLVE_MODE_NONE &&
       !(gfx->rendering_flags & VK_RENDERING_SUSPENDING_BIT)) {
      const struct anv_image_view *src_iview = gfx->depth_att.iview;

      /* MSAA resolves sample from the source attachment.  Transition the
       * depth attachment first to get rid of any HiZ that we may not be
       * able to handle.
       */
      transition_depth_buffer(cmd_buffer, src_iview->image,
                              src_iview->planes[0].isl.base_array_layer,
                              layers,
                              gfx->depth_att.layout,
                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              false /* will_full_fast_clear */);

      cmd_buffer_resolve_msaa_attachment(cmd_buffer, &gfx->depth_att,
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         VK_IMAGE_ASPECT_DEPTH_BIT);

      /* Transition the source back to the original layout.  This seems a bit
       * inefficient but, since HiZ resolves aren't destructive, going from
       * less HiZ to more is generally a no-op.
       */
      transition_depth_buffer(cmd_buffer, src_iview->image,
                              src_iview->planes[0].isl.base_array_layer,
                              layers,
                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              gfx->depth_att.layout,
                              false /* will_full_fast_clear */);
   }

   if (gfx->stencil_att.resolve_mode != VK_RESOLVE_MODE_NONE &&
       !(gfx->rendering_flags & VK_RENDERING_SUSPENDING_BIT)) {
      cmd_buffer_resolve_msaa_attachment(cmd_buffer, &gfx->stencil_att,
                                         gfx->stencil_att.layout,
                                         VK_IMAGE_ASPECT_STENCIL_BIT);
   }

#if GFX_VER == 7
   /* On gfx7, we have to store a texturable version of the stencil buffer in
    * a shadow whenever VK_IMAGE_USAGE_SAMPLED_BIT is set and copy back and
    * forth at strategic points. Stencil writes are only allowed in following
    * layouts:
    *
    *  - VK_IMAGE_LAYOUT_GENERAL
    *  - VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    *  - VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    *  - VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
    *  - VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
    *  - VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    *  - VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT
    *
    * For general, we have no nice opportunity to transition so we do the copy
    * to the shadow unconditionally at the end of the subpass. For transfer
    * destinations, we can update it as part of the transfer op. For the other
    * layouts, we delay the copy until a transition into some other layout.
    */
   if (gfx->stencil_att.iview != NULL) {
      const struct anv_image_view *iview = gfx->stencil_att.iview;
      const struct anv_image *image = iview->image;
      const uint32_t plane =
         anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_STENCIL_BIT);

      if (anv_surface_is_valid(&image->planes[plane].shadow_surface) &&
          (gfx->stencil_att.layout == VK_IMAGE_LAYOUT_GENERAL ||
           gfx->stencil_att.layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT)) {
         anv_image_copy_to_shadow(cmd_buffer, image,
                                  VK_IMAGE_ASPECT_STENCIL_BIT,
                                  iview->planes[plane].isl.base_level, 1,
                                  iview->planes[plane].isl.base_array_layer,
                                  layers);
      }
   }
#endif

   anv_cmd_buffer_reset_rendering(cmd_buffer);
}

void
genX(cmd_emit_conditional_render_predicate)(struct anv_cmd_buffer *cmd_buffer)
{
#if GFX_VERx10 >= 75
   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   mi_store(&b, mi_reg64(MI_PREDICATE_SRC0),
                mi_reg32(ANV_PREDICATE_RESULT_REG));
   mi_store(&b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(0));

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOADINV;
      mip.CombineOperation = COMBINE_SET;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }
#endif
}

#if GFX_VERx10 >= 75
void genX(CmdBeginConditionalRenderingEXT)(
   VkCommandBuffer                             commandBuffer,
   const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, buffer, pConditionalRenderingBegin->buffer);
   struct anv_cmd_state *cmd_state = &cmd_buffer->state;
   struct anv_address value_address =
      anv_address_add(buffer->address, pConditionalRenderingBegin->offset);

   const bool isInverted = pConditionalRenderingBegin->flags &
                           VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT;

   cmd_state->conditional_render_enabled = true;

   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   /* Section 19.4 of the Vulkan 1.1.85 spec says:
    *
    *    If the value of the predicate in buffer memory changes
    *    while conditional rendering is active, the rendering commands
    *    may be discarded in an implementation-dependent way.
    *    Some implementations may latch the value of the predicate
    *    upon beginning conditional rendering while others
    *    may read it before every rendering command.
    *
    * So it's perfectly fine to read a value from the buffer once.
    */
   struct mi_value value =  mi_mem32(value_address);

   /* Precompute predicate result, it is necessary to support secondary
    * command buffers since it is unknown if conditional rendering is
    * inverted when populating them.
    */
   mi_store(&b, mi_reg64(ANV_PREDICATE_RESULT_REG),
                isInverted ? mi_uge(&b, mi_imm(0), value) :
                             mi_ult(&b, mi_imm(0), value));
}

void genX(CmdEndConditionalRenderingEXT)(
	VkCommandBuffer                             commandBuffer)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_cmd_state *cmd_state = &cmd_buffer->state;

   cmd_state->conditional_render_enabled = false;
}
#endif

/* Set of stage bits for which are pipelined, i.e. they get queued
 * by the command streamer for later execution.
 */
#define ANV_PIPELINE_STAGE_PIPELINED_BITS \
   ~(VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT | \
     VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | \
     VK_PIPELINE_STAGE_2_HOST_BIT | \
     VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT)

void genX(CmdSetEvent2)(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     _event,
    const VkDependencyInfo*                     pDependencyInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_event, event, _event);

   VkPipelineStageFlags2 src_stages = 0;

   for (uint32_t i = 0; i < pDependencyInfo->memoryBarrierCount; i++)
      src_stages |= pDependencyInfo->pMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->bufferMemoryBarrierCount; i++)
      src_stages |= pDependencyInfo->pBufferMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; i++)
      src_stages |= pDependencyInfo->pImageMemoryBarriers[i].srcStageMask;

   cmd_buffer->state.pending_pipe_bits |= ANV_PIPE_POST_SYNC_BIT;
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      if (src_stages & ANV_PIPELINE_STAGE_PIPELINED_BITS) {
         pc.StallAtPixelScoreboard = true;
         pc.CommandStreamerStallEnable = true;
      }

      pc.DestinationAddressType  = DAT_PPGTT,
      pc.PostSyncOperation       = WriteImmediateData,
      pc.Address = (struct anv_address) {
         cmd_buffer->device->dynamic_state_pool.block_pool.bo,
         event->state.offset
      };
      pc.ImmediateData           = VK_EVENT_SET;
      anv_debug_dump_pc(pc);
   }
}

void genX(CmdResetEvent2)(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     _event,
    VkPipelineStageFlags2                       stageMask)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_event, event, _event);

   cmd_buffer->state.pending_pipe_bits |= ANV_PIPE_POST_SYNC_BIT;
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      if (stageMask & ANV_PIPELINE_STAGE_PIPELINED_BITS) {
         pc.StallAtPixelScoreboard = true;
         pc.CommandStreamerStallEnable = true;
      }

      pc.DestinationAddressType  = DAT_PPGTT;
      pc.PostSyncOperation       = WriteImmediateData;
      pc.Address = (struct anv_address) {
         cmd_buffer->device->dynamic_state_pool.block_pool.bo,
         event->state.offset
      };
      pc.ImmediateData           = VK_EVENT_RESET;
      anv_debug_dump_pc(pc);
   }
}

void genX(CmdWaitEvents2)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

#if GFX_VER >= 8
   for (uint32_t i = 0; i < eventCount; i++) {
      ANV_FROM_HANDLE(anv_event, event, pEvents[i]);

      anv_batch_emit(&cmd_buffer->batch, GENX(MI_SEMAPHORE_WAIT), sem) {
         sem.WaitMode            = PollingMode,
         sem.CompareOperation    = COMPARE_SAD_EQUAL_SDD,
         sem.SemaphoreDataDword  = VK_EVENT_SET,
         sem.SemaphoreAddress = (struct anv_address) {
            cmd_buffer->device->dynamic_state_pool.block_pool.bo,
            event->state.offset
         };
      }
   }
#else
   anv_finishme("Implement events on gfx7");
#endif

   cmd_buffer_barrier(cmd_buffer, pDependencyInfos, "wait event");
}

static uint32_t vk_to_intel_index_type(VkIndexType type)
{
   switch (type) {
   case VK_INDEX_TYPE_UINT8_EXT:
      return INDEX_BYTE;
   case VK_INDEX_TYPE_UINT16:
      return INDEX_WORD;
   case VK_INDEX_TYPE_UINT32:
      return INDEX_DWORD;
   default:
      unreachable("invalid index type");
   }
}

static uint32_t restart_index_for_type(VkIndexType type)
{
   switch (type) {
   case VK_INDEX_TYPE_UINT8_EXT:
      return UINT8_MAX;
   case VK_INDEX_TYPE_UINT16:
      return UINT16_MAX;
   case VK_INDEX_TYPE_UINT32:
      return UINT32_MAX;
   default:
      unreachable("invalid index type");
   }
}

void genX(CmdBindIndexBuffer)(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    _buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, buffer, _buffer);

   cmd_buffer->state.gfx.restart_index = restart_index_for_type(indexType);
   cmd_buffer->state.gfx.index_buffer = buffer;
   cmd_buffer->state.gfx.index_type = vk_to_intel_index_type(indexType);
   cmd_buffer->state.gfx.index_offset = offset;

   cmd_buffer->state.gfx.dirty |= ANV_CMD_DIRTY_INDEX_BUFFER;
}

VkResult genX(CmdSetPerformanceOverrideINTEL)(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   switch (pOverrideInfo->type) {
   case VK_PERFORMANCE_OVERRIDE_TYPE_NULL_HARDWARE_INTEL: {
      anv_batch_write_reg(&cmd_buffer->batch, GENX(INSTPM), instpm) {
         instpm._3DRenderingInstructionDisable = pOverrideInfo->enable;
         instpm.MediaInstructionDisable = pOverrideInfo->enable;
         instpm._3DRenderingInstructionDisableMask = true;
         instpm.MediaInstructionDisableMask = true;
      }
      break;
   }

   case VK_PERFORMANCE_OVERRIDE_TYPE_FLUSH_GPU_CACHES_INTEL:
      if (pOverrideInfo->enable) {
         /* FLUSH ALL THE THINGS! As requested by the MDAPI team. */
         anv_add_pending_pipe_bits(cmd_buffer,
                                   ANV_PIPE_FLUSH_BITS |
                                   ANV_PIPE_INVALIDATE_BITS,
                                   "perf counter isolation");
         genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
      }
      break;

   default:
      unreachable("Invalid override");
   }

   return VK_SUCCESS;
}

VkResult genX(CmdSetPerformanceStreamMarkerINTEL)(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo)
{
   /* TODO: Waiting on the register to write, might depend on generation. */

   return VK_SUCCESS;
}

#define TIMESTAMP 0x2358

void genX(cmd_emit_timestamp)(struct anv_batch *batch,
                              struct anv_device *device,
                              struct anv_address addr,
                              enum anv_timestamp_capture_type type) {
   switch (type) {
   case ANV_TIMESTAMP_CAPTURE_TOP_OF_PIPE: {
      struct mi_builder b;
      mi_builder_init(&b, device->info, batch);
      mi_store(&b, mi_mem64(addr), mi_reg64(TIMESTAMP));
      break;
   }

   case ANV_TIMESTAMP_CAPTURE_END_OF_PIPE:
      anv_batch_emit(batch, GENX(PIPE_CONTROL), pc) {
         pc.PostSyncOperation   = WriteTimestamp;
         pc.Address             = addr;
         anv_debug_dump_pc(pc);
      }
      break;

   case ANV_TIMESTAMP_CAPTURE_AT_CS_STALL:
      anv_batch_emit(batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.PostSyncOperation    = WriteTimestamp;
         pc.Address              = addr;
         anv_debug_dump_pc(pc);
      }
      break;

   default:
      unreachable("invalid");
   }
}
