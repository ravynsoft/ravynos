/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#include "tu_cmd_buffer.h"

#include "vk_render_pass.h"
#include "vk_util.h"
#include "vk_common_entrypoints.h"

#include "tu_clear_blit.h"
#include "tu_cs.h"
#include "tu_image.h"
#include "tu_tracepoints.h"

#include "common/freedreno_gpu_event.h"

static void
tu_clone_trace_range(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                     struct u_trace_iterator begin, struct u_trace_iterator end)
{
   if (u_trace_iterator_equal(begin, end))
      return;

   tu_cs_emit_wfi(cs);
   tu_cs_emit_pkt7(cs, CP_WAIT_FOR_ME, 0);
   u_trace_clone_append(begin, end, &cmd->trace, cs,
         tu_copy_timestamp_buffer);
}

static void
tu_clone_trace(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
               struct u_trace *trace)
{
   tu_clone_trace_range(cmd, cs, u_trace_begin_iterator(trace),
         u_trace_end_iterator(trace));
}

template <chip CHIP>
static void
tu_emit_raw_event_write(struct tu_cmd_buffer *cmd,
                        struct tu_cs *cs,
                        enum vgt_event_type event,
                        bool needs_seqno)
{
   if (CHIP == A6XX) {
      tu_cs_emit_pkt7(cs, CP_EVENT_WRITE, needs_seqno ? 4 : 1);
      tu_cs_emit(cs, CP_EVENT_WRITE_0_EVENT(event));
   } else {
      tu_cs_emit_pkt7(cs, CP_EVENT_WRITE7, needs_seqno ? 4 : 1);
      tu_cs_emit(cs,
         CP_EVENT_WRITE7_0(.event = event,
                           .write_src = EV_WRITE_USER_32B,
                           .write_dst = EV_DST_RAM,
                           .write_enabled = needs_seqno).value);
   }

   if (needs_seqno) {
      tu_cs_emit_qw(cs, global_iova(cmd, seqno_dummy));
      tu_cs_emit(cs, 0);
   }
}

template <chip CHIP>
void
tu_emit_event_write(struct tu_cmd_buffer *cmd,
                    struct tu_cs *cs,
                    enum fd_gpu_event event)
{
   struct fd_gpu_event_info event_info = fd_gpu_events<CHIP>[event];
   tu_emit_raw_event_write<CHIP>(cmd, cs, event_info.raw_event,
                                 event_info.needs_seqno);
}
TU_GENX(tu_emit_event_write);

/* Emits the tessfactor address to the top-level CS if it hasn't been already.
 * Updating this register requires a WFI if outstanding drawing is using it, but
 * tu6_init_hardware() will have WFIed before we started and no other draws
 * could be using the tessfactor address yet since we only emit one per cmdbuf.
 */
template <chip CHIP>
static void
tu6_lazy_emit_tessfactor_addr(struct tu_cmd_buffer *cmd)
{
   if (cmd->state.tessfactor_addr_set)
      return;

   tu_cs_emit_regs(&cmd->cs, PC_TESSFACTOR_ADDR(CHIP, .qword = cmd->device->tess_bo->iova));
   /* Updating PC_TESSFACTOR_ADDR could race with the next draw which uses it. */
   cmd->state.cache.flush_bits |= TU_CMD_FLAG_WAIT_FOR_IDLE;
   cmd->state.tessfactor_addr_set = true;
}

static void
tu6_lazy_emit_vsc(struct tu_cmd_buffer *cmd, struct tu_cs *cs)
{
   struct tu_device *dev = cmd->device;
   uint32_t num_vsc_pipes = dev->physical_device->info->num_vsc_pipes;

   /* VSC buffers:
    * use vsc pitches from the largest values used so far with this device
    * if there hasn't been overflow, there will already be a scratch bo
    * allocated for these sizes
    *
    * if overflow is detected, the stream size is increased by 2x
    */
   mtx_lock(&dev->mutex);

   struct tu6_global *global = dev->global_bo_map;

   uint32_t vsc_draw_overflow = global->vsc_draw_overflow;
   uint32_t vsc_prim_overflow = global->vsc_prim_overflow;

   if (vsc_draw_overflow >= dev->vsc_draw_strm_pitch)
      dev->vsc_draw_strm_pitch = (dev->vsc_draw_strm_pitch - VSC_PAD) * 2 + VSC_PAD;

   if (vsc_prim_overflow >= dev->vsc_prim_strm_pitch)
      dev->vsc_prim_strm_pitch = (dev->vsc_prim_strm_pitch - VSC_PAD) * 2 + VSC_PAD;

   cmd->vsc_prim_strm_pitch = dev->vsc_prim_strm_pitch;
   cmd->vsc_draw_strm_pitch = dev->vsc_draw_strm_pitch;

   mtx_unlock(&dev->mutex);

   struct tu_bo *vsc_bo;
   uint32_t size0 = cmd->vsc_prim_strm_pitch * num_vsc_pipes +
                    cmd->vsc_draw_strm_pitch * num_vsc_pipes;

   tu_get_scratch_bo(dev, size0 + num_vsc_pipes * 4, &vsc_bo);

   tu_cs_emit_regs(cs,
                   A6XX_VSC_DRAW_STRM_SIZE_ADDRESS(.bo = vsc_bo, .bo_offset = size0));
   tu_cs_emit_regs(cs,
                   A6XX_VSC_PRIM_STRM_ADDRESS(.bo = vsc_bo));
   tu_cs_emit_regs(
      cs, A6XX_VSC_DRAW_STRM_ADDRESS(.bo = vsc_bo,
                                     .bo_offset = cmd->vsc_prim_strm_pitch *
                                                  num_vsc_pipes));

   cmd->vsc_initialized = true;
}

template <chip CHIP>
static void
tu6_emit_flushes(struct tu_cmd_buffer *cmd_buffer,
                 struct tu_cs *cs,
                 struct tu_cache_state *cache)
{
   BITMASK_ENUM(tu_cmd_flush_bits) flushes = cache->flush_bits;
   cache->flush_bits = 0;

   if (TU_DEBUG(FLUSHALL))
      flushes |= TU_CMD_FLAG_ALL_FLUSH | TU_CMD_FLAG_ALL_INVALIDATE;

   if (TU_DEBUG(SYNCDRAW))
      flushes |= TU_CMD_FLAG_WAIT_MEM_WRITES |
                 TU_CMD_FLAG_WAIT_FOR_IDLE |
                 TU_CMD_FLAG_WAIT_FOR_ME;

   /* Experiments show that invalidating CCU while it still has data in it
    * doesn't work, so make sure to always flush before invalidating in case
    * any data remains that hasn't yet been made available through a barrier.
    * However it does seem to work for UCHE.
    */
   if (flushes & (TU_CMD_FLAG_CCU_FLUSH_COLOR |
                  TU_CMD_FLAG_CCU_INVALIDATE_COLOR))
      tu_emit_event_write<CHIP>(cmd_buffer, cs, FD_CCU_FLUSH_COLOR);
   if (flushes & (TU_CMD_FLAG_CCU_FLUSH_DEPTH |
                  TU_CMD_FLAG_CCU_INVALIDATE_DEPTH))
      tu_emit_event_write<CHIP>(cmd_buffer, cs, FD_CCU_FLUSH_DEPTH);
   if (flushes & TU_CMD_FLAG_CCU_INVALIDATE_COLOR)
      tu_emit_event_write<CHIP>(cmd_buffer, cs, FD_CCU_INVALIDATE_COLOR);
   if (flushes & TU_CMD_FLAG_CCU_INVALIDATE_DEPTH)
      tu_emit_event_write<CHIP>(cmd_buffer, cs, FD_CCU_INVALIDATE_DEPTH);
   if (flushes & TU_CMD_FLAG_CACHE_FLUSH)
      tu_emit_event_write<CHIP>(cmd_buffer, cs, FD_CACHE_FLUSH);
   if (flushes & TU_CMD_FLAG_CACHE_INVALIDATE)
      tu_emit_event_write<CHIP>(cmd_buffer, cs, FD_CACHE_INVALIDATE);
   if (flushes & TU_CMD_FLAG_BINDLESS_DESCRIPTOR_INVALIDATE) {
      tu_cs_emit_regs(cs, HLSQ_INVALIDATE_CMD(CHIP,
            .cs_bindless = CHIP == A6XX ? 0x1f : 0xff,
            .gfx_bindless = CHIP == A6XX ? 0x1f : 0xff,
      ));
   }
   if (flushes & TU_CMD_FLAG_WAIT_MEM_WRITES)
      tu_cs_emit_pkt7(cs, CP_WAIT_MEM_WRITES, 0);
   if (flushes & TU_CMD_FLAG_WAIT_FOR_IDLE)
      tu_cs_emit_wfi(cs);
   if (flushes & TU_CMD_FLAG_WAIT_FOR_ME)
      tu_cs_emit_pkt7(cs, CP_WAIT_FOR_ME, 0);
}

/* "Normal" cache flushes outside the renderpass, that don't require any special handling */
template <chip CHIP>
void
tu_emit_cache_flush(struct tu_cmd_buffer *cmd_buffer)
{
   tu6_emit_flushes<CHIP>(cmd_buffer, &cmd_buffer->cs, &cmd_buffer->state.cache);
}
TU_GENX(tu_emit_cache_flush);

/* Renderpass cache flushes inside the draw_cs */
template <chip CHIP>
void
tu_emit_cache_flush_renderpass(struct tu_cmd_buffer *cmd_buffer)
{
   if (!cmd_buffer->state.renderpass_cache.flush_bits &&
       likely(!tu_env.debug))
      return;
   tu6_emit_flushes<CHIP>(cmd_buffer, &cmd_buffer->draw_cs,
                    &cmd_buffer->state.renderpass_cache);
}
TU_GENX(tu_emit_cache_flush_renderpass);

template <chip CHIP>
static struct fd_reg_pair
rb_ccu_cntl(struct tu_device *dev, bool gmem)
{
   if (CHIP == A7XX) {
      return A6XX_RB_CCU_CNTL(.dword = gmem ? 0x68 : 0);
   }

   uint32_t color_offset = gmem ? dev->physical_device->ccu_offset_gmem
                                : dev->physical_device->ccu_offset_bypass;

   uint32_t color_offset_hi = color_offset >> 21;
   color_offset &= 0x1fffff;
   enum a6xx_ccu_color_cache_size cache_size =
      (a6xx_ccu_color_cache_size)(dev->physical_device->info->a6xx.gmem_ccu_color_cache_fraction);
   bool concurrent_resolve = dev->physical_device->info->a6xx.concurrent_resolve;
   return  A6XX_RB_CCU_CNTL(.gmem_fast_clear_disable =
         !dev->physical_device->info->a6xx.has_gmem_fast_clear,
      .concurrent_resolve = concurrent_resolve,
      .depth_offset_hi = 0,
      .color_offset_hi = color_offset_hi,
      .depth_cache_size = 0,
      .depth_offset = 0,
      .color_cache_size = cache_size,
      .color_offset = color_offset);
}

/* Cache flushes for things that use the color/depth read/write path (i.e.
 * blits and draws). This deals with changing CCU state as well as the usual
 * cache flushing.
 */
template <chip CHIP>
void
tu_emit_cache_flush_ccu(struct tu_cmd_buffer *cmd_buffer,
                        struct tu_cs *cs,
                        enum tu_cmd_ccu_state ccu_state)
{
   assert(ccu_state != TU_CMD_CCU_UNKNOWN);
   /* It's unsafe to flush inside condition because we clear flush_bits */
   assert(!cs->cond_stack_depth);

   /* Changing CCU state must involve invalidating the CCU. In sysmem mode,
    * the CCU may also contain data that we haven't flushed out yet, so we
    * also need to flush. Also, in order to program RB_CCU_CNTL, we need to
    * emit a WFI as it isn't pipelined.
    */
   if (ccu_state != cmd_buffer->state.ccu_state) {
      if (cmd_buffer->state.ccu_state != TU_CMD_CCU_GMEM) {
         cmd_buffer->state.cache.flush_bits |=
            TU_CMD_FLAG_CCU_FLUSH_COLOR |
            TU_CMD_FLAG_CCU_FLUSH_DEPTH;
         cmd_buffer->state.cache.pending_flush_bits &= ~(
            TU_CMD_FLAG_CCU_FLUSH_COLOR |
            TU_CMD_FLAG_CCU_FLUSH_DEPTH);
      }
      cmd_buffer->state.cache.flush_bits |=
         TU_CMD_FLAG_CCU_INVALIDATE_COLOR |
         TU_CMD_FLAG_CCU_INVALIDATE_DEPTH |
         TU_CMD_FLAG_WAIT_FOR_IDLE;
      cmd_buffer->state.cache.pending_flush_bits &= ~(
         TU_CMD_FLAG_CCU_INVALIDATE_COLOR |
         TU_CMD_FLAG_CCU_INVALIDATE_DEPTH |
         TU_CMD_FLAG_WAIT_FOR_IDLE);
   }

   tu6_emit_flushes<CHIP>(cmd_buffer, cs, &cmd_buffer->state.cache);

   if (ccu_state != cmd_buffer->state.ccu_state) {
      tu_cs_emit_regs(cs, rb_ccu_cntl<CHIP>(cmd_buffer->device,
                                            ccu_state == TU_CMD_CCU_GMEM));
      cmd_buffer->state.ccu_state = ccu_state;
   }
}
TU_GENX(tu_emit_cache_flush_ccu);

template <chip CHIP>
static void
tu6_emit_zs(struct tu_cmd_buffer *cmd,
            const struct tu_subpass *subpass,
            struct tu_cs *cs)
{
   const uint32_t a = subpass->depth_stencil_attachment.attachment;
   if (a == VK_ATTACHMENT_UNUSED) {
      tu_cs_emit_regs(cs,
                      A6XX_RB_DEPTH_BUFFER_INFO(.depth_format = DEPTH6_NONE),
                      A6XX_RB_DEPTH_BUFFER_PITCH(0),
                      A6XX_RB_DEPTH_BUFFER_ARRAY_PITCH(0),
                      A6XX_RB_DEPTH_BUFFER_BASE(0),
                      A6XX_RB_DEPTH_BUFFER_BASE_GMEM(0));

      tu_cs_emit_regs(cs,
                      A6XX_GRAS_SU_DEPTH_BUFFER_INFO(.depth_format = DEPTH6_NONE));

      tu_cs_emit_regs(cs, A6XX_RB_STENCIL_INFO(0));

      return;
   }

   const struct tu_image_view *iview = cmd->state.attachments[a];
   const struct tu_render_pass_attachment *attachment =
      &cmd->state.pass->attachments[a];
   enum a6xx_depth_format fmt = tu6_pipe2depth(attachment->format);

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_DEPTH_BUFFER_INFO, 6);
   tu_cs_emit(cs, RB_DEPTH_BUFFER_INFO(CHIP,
                     .depth_format = fmt,
                     .tilemode = TILE6_3,
                     .losslesscompen = iview->view.ubwc_enabled,
                     ).value);
   if (attachment->format == VK_FORMAT_D32_SFLOAT_S8_UINT)
      tu_cs_image_depth_ref(cs, iview, 0);
   else
      tu_cs_image_ref(cs, &iview->view, 0);
   tu_cs_emit(cs, tu_attachment_gmem_offset(cmd, attachment, 0));

   tu_cs_emit_regs(cs,
                   A6XX_GRAS_SU_DEPTH_BUFFER_INFO(.depth_format = fmt));

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_DEPTH_FLAG_BUFFER_BASE, 3);
   tu_cs_image_flag_ref(cs, &iview->view, 0);

   if (attachment->format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
       attachment->format == VK_FORMAT_S8_UINT) {

      tu_cs_emit_pkt4(cs, REG_A6XX_RB_STENCIL_INFO, 6);
      tu_cs_emit(cs, RB_STENCIL_INFO(CHIP,
                        .separate_stencil = true,
                        .tilemode = TILE6_3,
                        ).value);
      if (attachment->format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
         tu_cs_image_stencil_ref(cs, iview, 0);
         tu_cs_emit(cs, tu_attachment_gmem_offset_stencil(cmd, attachment, 0));
      } else {
         tu_cs_image_ref(cs, &iview->view, 0);
         tu_cs_emit(cs, tu_attachment_gmem_offset(cmd, attachment, 0));
      }
   } else {
      tu_cs_emit_regs(cs,
                     A6XX_RB_STENCIL_INFO(0));
   }
}

static void
tu6_emit_mrt(struct tu_cmd_buffer *cmd,
             const struct tu_subpass *subpass,
             struct tu_cs *cs)
{
   const struct tu_framebuffer *fb = cmd->state.framebuffer;

   enum a6xx_format mrt0_format = FMT6_NONE;

   for (uint32_t i = 0; i < subpass->color_count; ++i) {
      uint32_t a = subpass->color_attachments[i].attachment;
      if (a == VK_ATTACHMENT_UNUSED) {
         /* From the VkPipelineRenderingCreateInfo definition:
          *
          *    Valid formats indicate that an attachment can be used - but it
          *    is still valid to set the attachment to NULL when beginning
          *    rendering.
          *
          * This means that with dynamic rendering, pipelines may write to
          * some attachments that are UNUSED here. Setting the format to 0
          * here should prevent them from writing to anything. This also seems
          * to also be required for alpha-to-coverage which can use the alpha
          * value for an otherwise-unused attachment.
          */
         tu_cs_emit_pkt4(cs, REG_A6XX_RB_MRT_BUF_INFO(i), 6);
         for (unsigned i = 0; i < 6; i++)
            tu_cs_emit(cs, 0);

         tu_cs_emit_regs(cs,
                         A6XX_SP_FS_MRT_REG(i, .dword = 0));
         continue;
      }

      const struct tu_image_view *iview = cmd->state.attachments[a];

      tu_cs_emit_pkt4(cs, REG_A6XX_RB_MRT_BUF_INFO(i), 6);
      tu_cs_emit(cs, iview->view.RB_MRT_BUF_INFO);
      tu_cs_image_ref(cs, &iview->view, 0);
      tu_cs_emit(cs, tu_attachment_gmem_offset(cmd, &cmd->state.pass->attachments[a], 0));

      tu_cs_emit_regs(cs,
                      A6XX_SP_FS_MRT_REG(i, .dword = iview->view.SP_FS_MRT_REG));

      tu_cs_emit_pkt4(cs, REG_A6XX_RB_MRT_FLAG_BUFFER_ADDR(i), 3);
      tu_cs_image_flag_ref(cs, &iview->view, 0);

      if (i == 0)
         mrt0_format = (enum a6xx_format) (iview->view.SP_FS_MRT_REG & 0xff);
   }

   tu_cs_emit_regs(cs, A6XX_GRAS_LRZ_MRT_BUF_INFO_0(.color_format = mrt0_format));

   tu_cs_emit_regs(cs,
                   A6XX_RB_SRGB_CNTL(.dword = subpass->srgb_cntl));
   tu_cs_emit_regs(cs,
                   A6XX_SP_SRGB_CNTL(.dword = subpass->srgb_cntl));

   unsigned layers = MAX2(fb->layers, util_logbase2(subpass->multiview_mask) + 1);
   tu_cs_emit_regs(cs, A6XX_GRAS_MAX_LAYER_INDEX(layers - 1));
}

struct tu_bin_size_params {
   enum a6xx_render_mode render_mode;
   bool force_lrz_write_dis;
   enum a6xx_buffers_location buffers_location;
   unsigned lrz_feedback_zmode_mask;
};

template <chip CHIP>
static void
tu6_emit_bin_size(struct tu_cs *cs,
                  uint32_t bin_w,
                  uint32_t bin_h,
                  struct tu_bin_size_params &&p)
{
   if (CHIP == A6XX) {
      tu_cs_emit_regs(
         cs, A6XX_GRAS_BIN_CONTROL(.binw = bin_w,
                                   .binh = bin_h,
                                   .render_mode = p.render_mode,
                                   .force_lrz_write_dis = p.force_lrz_write_dis,
                                   .buffers_location = p.buffers_location,
                                   .lrz_feedback_zmode_mask = p.lrz_feedback_zmode_mask, ));
   } else {
      tu_cs_emit_regs(cs,
                      A6XX_GRAS_BIN_CONTROL(.binw = bin_w,
                                            .binh = bin_h,
                                            .render_mode = p.render_mode,
                                            .force_lrz_write_dis = p.force_lrz_write_dis,
                                            .lrz_feedback_zmode_mask =
                                               p.lrz_feedback_zmode_mask, ));
   }

   tu_cs_emit_regs(cs, RB_BIN_CONTROL(CHIP,
                        .binw = bin_w,
                        .binh = bin_h,
                        .render_mode = p.render_mode,
                        .force_lrz_write_dis = p.force_lrz_write_dis,
                        .buffers_location = p.buffers_location,
                        .lrz_feedback_zmode_mask = p.lrz_feedback_zmode_mask, ));

   /* no flag for RB_BIN_CONTROL2... */
   tu_cs_emit_regs(cs,
                   A6XX_RB_BIN_CONTROL2(.binw = bin_w,
                                        .binh = bin_h));
}

template <chip CHIP>
static void
tu6_emit_render_cntl(struct tu_cmd_buffer *cmd,
                     const struct tu_subpass *subpass,
                     struct tu_cs *cs,
                     bool binning);

template <>
void
tu6_emit_render_cntl<A6XX>(struct tu_cmd_buffer *cmd,
                     const struct tu_subpass *subpass,
                     struct tu_cs *cs,
                     bool binning)
{
   /* doesn't RB_RENDER_CNTL set differently for binning pass: */
   bool no_track = !cmd->device->physical_device->info->a6xx.has_cp_reg_write;
   uint32_t cntl = 0;
   cntl |= A6XX_RB_RENDER_CNTL_CCUSINGLECACHELINESIZE(2);
   if (binning) {
      if (no_track)
         return;
      cntl |= A6XX_RB_RENDER_CNTL_BINNING;
   } else {
      uint32_t mrts_ubwc_enable = 0;
      for (uint32_t i = 0; i < subpass->color_count; ++i) {
         uint32_t a = subpass->color_attachments[i].attachment;
         if (a == VK_ATTACHMENT_UNUSED)
            continue;

         const struct tu_image_view *iview = cmd->state.attachments[a];
         if (iview->view.ubwc_enabled)
            mrts_ubwc_enable |= 1 << i;
      }

      cntl |= A6XX_RB_RENDER_CNTL_FLAG_MRTS(mrts_ubwc_enable);

      const uint32_t a = subpass->depth_stencil_attachment.attachment;
      if (a != VK_ATTACHMENT_UNUSED) {
         const struct tu_image_view *iview = cmd->state.attachments[a];
         if (iview->view.ubwc_enabled)
            cntl |= A6XX_RB_RENDER_CNTL_FLAG_DEPTH;
      }

      if (no_track) {
         tu_cs_emit_pkt4(cs, REG_A6XX_RB_RENDER_CNTL, 1);
         tu_cs_emit(cs, cntl);
         return;
      }

      /* In the !binning case, we need to set RB_RENDER_CNTL in the draw_cs
       * in order to set it correctly for the different subpasses. However,
       * that means the packets we're emitting also happen during binning. So
       * we need to guard the write on !BINNING at CP execution time.
       */
      tu_cs_reserve(cs, 3 + 4);
      tu_cs_emit_pkt7(cs, CP_COND_REG_EXEC, 2);
      tu_cs_emit(cs, CP_COND_REG_EXEC_0_MODE(RENDER_MODE) |
                     CP_COND_REG_EXEC_0_GMEM | CP_COND_REG_EXEC_0_SYSMEM);
      tu_cs_emit(cs, RENDER_MODE_CP_COND_REG_EXEC_1_DWORDS(4));
   }

   tu_cs_emit_pkt7(cs, CP_REG_WRITE, 3);
   tu_cs_emit(cs, CP_REG_WRITE_0_TRACKER(TRACK_RENDER_CNTL));
   tu_cs_emit(cs, REG_A6XX_RB_RENDER_CNTL);
   tu_cs_emit(cs, cntl);
}

template <>
void
tu6_emit_render_cntl<A7XX>(struct tu_cmd_buffer *cmd,
                     const struct tu_subpass *subpass,
                     struct tu_cs *cs,
                     bool binning)
{
   tu_cs_emit_regs(
      cs, A7XX_RB_RENDER_CNTL(.binning = binning, .raster_mode = TYPE_TILED,
                              .raster_direction = LR_TB));
}

static void
tu6_emit_blit_scissor(struct tu_cmd_buffer *cmd, struct tu_cs *cs, bool align)
{
   struct tu_physical_device *phys_dev = cmd->device->physical_device;
   const VkRect2D *render_area = &cmd->state.render_area;

   /* Avoid assertion fails with an empty render area at (0, 0) where the
    * subtraction below wraps around. Empty render areas should be forced to
    * the sysmem path by use_sysmem_rendering(). It's not even clear whether
    * an empty scissor here works, and the blob seems to force sysmem too as
    * it sets something wrong (non-empty) for the scissor.
    */
   if (render_area->extent.width == 0 ||
       render_area->extent.height == 0)
      return;

   uint32_t x1 = render_area->offset.x;
   uint32_t y1 = render_area->offset.y;
   uint32_t x2 = x1 + render_area->extent.width - 1;
   uint32_t y2 = y1 + render_area->extent.height - 1;

   if (align) {
      x1 = x1 & ~(phys_dev->info->gmem_align_w - 1);
      y1 = y1 & ~(phys_dev->info->gmem_align_h - 1);
      x2 = ALIGN_POT(x2 + 1, phys_dev->info->gmem_align_w) - 1;
      y2 = ALIGN_POT(y2 + 1, phys_dev->info->gmem_align_h) - 1;
   }

   tu_cs_emit_regs(cs,
                   A6XX_RB_BLIT_SCISSOR_TL(.x = x1, .y = y1),
                   A6XX_RB_BLIT_SCISSOR_BR(.x = x2, .y = y2));
}

void
tu6_emit_window_scissor(struct tu_cs *cs,
                        uint32_t x1,
                        uint32_t y1,
                        uint32_t x2,
                        uint32_t y2)
{
   tu_cs_emit_regs(cs,
                   A6XX_GRAS_SC_WINDOW_SCISSOR_TL(.x = x1, .y = y1),
                   A6XX_GRAS_SC_WINDOW_SCISSOR_BR(.x = x2, .y = y2));

   tu_cs_emit_regs(cs,
                   A6XX_GRAS_2D_RESOLVE_CNTL_1(.x = x1, .y = y1),
                   A6XX_GRAS_2D_RESOLVE_CNTL_2(.x = x2, .y = y2));
}

template <chip CHIP>
void
tu6_emit_window_offset(struct tu_cs *cs, uint32_t x1, uint32_t y1)
{
   tu_cs_emit_regs(cs,
                   A6XX_RB_WINDOW_OFFSET(.x = x1, .y = y1));

   tu_cs_emit_regs(cs,
                   A6XX_RB_WINDOW_OFFSET2(.x = x1, .y = y1));

   tu_cs_emit_regs(cs,
                   SP_WINDOW_OFFSET(CHIP, .x = x1, .y = y1));

   tu_cs_emit_regs(cs,
                   A6XX_SP_TP_WINDOW_OFFSET(.x = x1, .y = y1));
}

void
tu6_apply_depth_bounds_workaround(struct tu_device *device,
                                  uint32_t *rb_depth_cntl)
{
   if (!device->physical_device->info->a6xx.depth_bounds_require_depth_test_quirk)
      return;

   /* On some GPUs it is necessary to enable z test for depth bounds test when
    * UBWC is enabled. Otherwise, the GPU would hang. FUNC_ALWAYS is required to
    * pass z test. Relevant tests:
    *  dEQP-VK.pipeline.extended_dynamic_state.two_draws_dynamic.depth_bounds_test_disable
    *  dEQP-VK.dynamic_state.ds_state.depth_bounds_1
    */
   *rb_depth_cntl |= A6XX_RB_DEPTH_CNTL_Z_TEST_ENABLE |
                     A6XX_RB_DEPTH_CNTL_ZFUNC(FUNC_ALWAYS);
}

static void
tu_cs_emit_draw_state(struct tu_cs *cs, uint32_t id, struct tu_draw_state state)
{
   uint32_t enable_mask;
   switch (id) {
   case TU_DRAW_STATE_VS:
   case TU_DRAW_STATE_FS:
   case TU_DRAW_STATE_VPC:
   /* The blob seems to not enable this (DESC_SETS_LOAD) for binning, even
    * when resources would actually be used in the binning shader.
    * Presumably the overhead of prefetching the resources isn't
    * worth it.
    */
   case TU_DRAW_STATE_DESC_SETS_LOAD:
      enable_mask = CP_SET_DRAW_STATE__0_GMEM |
                    CP_SET_DRAW_STATE__0_SYSMEM;
      break;
   case TU_DRAW_STATE_VS_BINNING:
   case TU_DRAW_STATE_GS_BINNING:
      enable_mask = CP_SET_DRAW_STATE__0_BINNING;
      break;
   case TU_DRAW_STATE_INPUT_ATTACHMENTS_GMEM:
   case TU_DRAW_STATE_PRIM_MODE_GMEM:
      enable_mask = CP_SET_DRAW_STATE__0_GMEM;
      break;
   case TU_DRAW_STATE_INPUT_ATTACHMENTS_SYSMEM:
      enable_mask = CP_SET_DRAW_STATE__0_SYSMEM;
      break;
   case TU_DRAW_STATE_PRIM_MODE_SYSMEM:
      /* By also applying the state during binning we ensure that there
       * is no rotation applied, by previous A6XX_GRAS_SC_CNTL::rotation.
       */
      enable_mask =
         CP_SET_DRAW_STATE__0_SYSMEM | CP_SET_DRAW_STATE__0_BINNING;
      break;
   default:
      enable_mask = CP_SET_DRAW_STATE__0_GMEM |
                    CP_SET_DRAW_STATE__0_SYSMEM |
                    CP_SET_DRAW_STATE__0_BINNING;
      break;
   }

   STATIC_ASSERT(TU_DRAW_STATE_COUNT <= 32);

   /* We need to reload the descriptors every time the descriptor sets
    * change. However, the commands we send only depend on the pipeline
    * because the whole point is to cache descriptors which are used by the
    * pipeline. There's a problem here, in that the firmware has an
    * "optimization" which skips executing groups that are set to the same
    * value as the last draw. This means that if the descriptor sets change
    * but not the pipeline, we'd try to re-execute the same buffer which
    * the firmware would ignore and we wouldn't pre-load the new
    * descriptors. Set the DIRTY bit to avoid this optimization.
    *
    * We set the dirty bit for shader draw states because they contain
    * CP_LOAD_STATE packets that are invalidated by the PROGRAM_CONFIG draw
    * state, so if PROGRAM_CONFIG changes but one of the shaders stays the
    * same then we still need to re-emit everything. The GLES blob which
    * implements separate shader draw states does the same thing.
    *
    * We also need to set this bit for draw states which may be patched by the
    * GPU, because their underlying memory may change between setting the draw
    * state.
    */
   if (id == TU_DRAW_STATE_DESC_SETS_LOAD ||
       id == TU_DRAW_STATE_VS ||
       id == TU_DRAW_STATE_VS_BINNING ||
       id == TU_DRAW_STATE_HS ||
       id == TU_DRAW_STATE_DS ||
       id == TU_DRAW_STATE_GS ||
       id == TU_DRAW_STATE_GS_BINNING ||
       id == TU_DRAW_STATE_FS ||
       state.writeable)
      enable_mask |= CP_SET_DRAW_STATE__0_DIRTY;

   tu_cs_emit(cs, CP_SET_DRAW_STATE__0_COUNT(state.size) |
                  enable_mask |
                  CP_SET_DRAW_STATE__0_GROUP_ID(id) |
                  COND(!state.size || !state.iova, CP_SET_DRAW_STATE__0_DISABLE));
   tu_cs_emit_qw(cs, state.iova);
}

void
tu6_emit_msaa(struct tu_cs *cs, VkSampleCountFlagBits vk_samples,
              bool msaa_disable)
{
   const enum a3xx_msaa_samples samples = tu_msaa_samples(vk_samples);
   msaa_disable |= (samples == MSAA_ONE);
   tu_cs_emit_regs(cs,
                   A6XX_SP_TP_RAS_MSAA_CNTL(samples),
                   A6XX_SP_TP_DEST_MSAA_CNTL(.samples = samples,
                                             .msaa_disable = msaa_disable));

   tu_cs_emit_regs(cs,
                   A6XX_GRAS_RAS_MSAA_CNTL(samples),
                   A6XX_GRAS_DEST_MSAA_CNTL(.samples = samples,
                                            .msaa_disable = msaa_disable));

   tu_cs_emit_regs(cs,
                   A6XX_RB_RAS_MSAA_CNTL(samples),
                   A6XX_RB_DEST_MSAA_CNTL(.samples = samples,
                                          .msaa_disable = msaa_disable));
}

static void
tu6_update_msaa(struct tu_cmd_buffer *cmd)
{
   VkSampleCountFlagBits samples =
      cmd->vk.dynamic_graphics_state.ms.rasterization_samples;;

   /* The samples may not be set by the pipeline or dynamically if raster
    * discard is enabled. We can set any valid value, but don't set the
    * default invalid value of 0.
    */
   if (samples == 0)
      samples = VK_SAMPLE_COUNT_1_BIT;
   tu6_emit_msaa(&cmd->draw_cs, samples, cmd->state.msaa_disable);
}

static void
tu6_update_msaa_disable(struct tu_cmd_buffer *cmd)
{
   VkPrimitiveTopology topology = 
      (VkPrimitiveTopology)cmd->vk.dynamic_graphics_state.ia.primitive_topology;
   bool is_line =
      topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST ||
      topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
      topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
      topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY ||
      (topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST &&
       cmd->state.shaders[MESA_SHADER_TESS_EVAL] &&
       cmd->state.shaders[MESA_SHADER_TESS_EVAL]->variant &&
       cmd->state.shaders[MESA_SHADER_TESS_EVAL]->variant->key.tessellation == IR3_TESS_ISOLINES);
   bool msaa_disable = is_line &&
      cmd->vk.dynamic_graphics_state.rs.line.mode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;

   if (cmd->state.msaa_disable != msaa_disable) {
      cmd->state.msaa_disable = msaa_disable;
      tu6_update_msaa(cmd);
   }
}

static bool
use_hw_binning(struct tu_cmd_buffer *cmd)
{
   const struct tu_framebuffer *fb = cmd->state.framebuffer;
   const struct tu_tiling_config *tiling = &fb->tiling[cmd->state.gmem_layout];

   /* XFB commands are emitted for BINNING || SYSMEM, which makes it
    * incompatible with non-hw binning GMEM rendering. this is required because
    * some of the XFB commands need to only be executed once.
    * use_sysmem_rendering() should have made sure we only ended up here if no
    * XFB was used.
    */
   if (cmd->state.rp.xfb_used) {
      assert(tiling->binning_possible);
      return true;
   }

   /* VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT emulates GL_PRIMITIVES_GENERATED,
    * which wasn't designed to care about tilers and expects the result not to
    * be multiplied by tile count.
    * See https://gitlab.khronos.org/vulkan/vulkan/-/issues/3131
    */
   if (cmd->state.rp.has_prim_generated_query_in_rp ||
       cmd->state.prim_generated_query_running_before_rp) {
      assert(tiling->binning_possible);
      return true;
   }

   return tiling->binning;
}

static bool
use_sysmem_rendering(struct tu_cmd_buffer *cmd,
                     struct tu_renderpass_result **autotune_result)
{
   if (TU_DEBUG(SYSMEM))
      return true;

   /* A7XX TODO: Add gmem support */
   if (cmd->device->physical_device->info->chip >= 7)
      return true;

   /* can't fit attachments into gmem */
   if (!cmd->state.tiling->possible)
      return true;

   if (cmd->state.framebuffer->layers > 1)
      return true;

   /* Use sysmem for empty render areas */
   if (cmd->state.render_area.extent.width == 0 ||
       cmd->state.render_area.extent.height == 0)
      return true;

   if (cmd->state.rp.has_tess)
      return true;

   if (cmd->state.rp.disable_gmem)
      return true;

   /* XFB is incompatible with non-hw binning GMEM rendering, see use_hw_binning */
   if (cmd->state.rp.xfb_used && !cmd->state.tiling->binning_possible)
      return true;

   /* QUERY_TYPE_PRIMITIVES_GENERATED is incompatible with non-hw binning
    * GMEM rendering, see use_hw_binning.
    */
   if ((cmd->state.rp.has_prim_generated_query_in_rp ||
        cmd->state.prim_generated_query_running_before_rp) &&
       !cmd->state.tiling->binning_possible)
      return true;

   if (TU_DEBUG(GMEM))
      return false;

   bool use_sysmem = tu_autotune_use_bypass(&cmd->device->autotune,
                                            cmd, autotune_result);
   if (*autotune_result) {
      list_addtail(&(*autotune_result)->node, &cmd->renderpass_autotune_results);
   }

   return use_sysmem;
}

/* Optimization: there is no reason to load gmem if there is no
 * geometry to process. COND_REG_EXEC predicate is set here,
 * but the actual skip happens in tu_load_gmem_attachment() and tile_store_cs,
 * for each blit separately.
 */
static void
tu6_emit_cond_for_load_stores(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                              uint32_t pipe, uint32_t slot, bool skip_wfm)
{
   if (cmd->state.tiling->binning_possible &&
       cmd->state.pass->has_cond_load_store) {
      tu_cs_emit_pkt7(cs, CP_REG_TEST, 1);
      tu_cs_emit(cs, A6XX_CP_REG_TEST_0_REG(REG_A6XX_VSC_STATE_REG(pipe)) |
                     A6XX_CP_REG_TEST_0_BIT(slot) |
                     COND(skip_wfm, A6XX_CP_REG_TEST_0_SKIP_WAIT_FOR_ME));
   } else {
      /* COND_REG_EXECs are not emitted in non-binning case */
   }
}

template <chip CHIP>
static void
tu6_emit_tile_select(struct tu_cmd_buffer *cmd,
                     struct tu_cs *cs,
                     uint32_t tx, uint32_t ty, uint32_t pipe, uint32_t slot,
                     const struct tu_image_view *fdm)
{
   const struct tu_tiling_config *tiling = cmd->state.tiling;

   tu_cs_emit_pkt7(cs, CP_SET_MARKER, 1);
   tu_cs_emit(cs, A6XX_CP_SET_MARKER_0_MODE(RM6_GMEM));

   const uint32_t x1 = tiling->tile0.width * tx;
   const uint32_t y1 = tiling->tile0.height * ty;
   const uint32_t x2 = MIN2(x1 + tiling->tile0.width, MAX_VIEWPORT_SIZE);
   const uint32_t y2 = MIN2(y1 + tiling->tile0.height, MAX_VIEWPORT_SIZE);
   tu6_emit_window_scissor(cs, x1, y1, x2 - 1, y2 - 1);
   tu6_emit_window_offset<CHIP>(cs, x1, y1);

   bool hw_binning = use_hw_binning(cmd);

   if (hw_binning) {
      tu_cs_emit_pkt7(cs, CP_WAIT_FOR_ME, 0);

      tu_cs_emit_pkt7(cs, CP_SET_MODE, 1);
      tu_cs_emit(cs, 0x0);

      tu_cs_emit_pkt7(cs, CP_SET_BIN_DATA5_OFFSET, 4);
      tu_cs_emit(cs, tiling->pipe_sizes[pipe] |
                     CP_SET_BIN_DATA5_0_VSC_N(slot));
      tu_cs_emit(cs, pipe * cmd->vsc_draw_strm_pitch);
      tu_cs_emit(cs, pipe * 4);
      tu_cs_emit(cs, pipe * cmd->vsc_prim_strm_pitch);
   }

   tu6_emit_cond_for_load_stores(cmd, cs, pipe, slot, hw_binning);

   tu_cs_emit_pkt7(cs, CP_SET_VISIBILITY_OVERRIDE, 1);
   tu_cs_emit(cs, !hw_binning);

   tu_cs_emit_pkt7(cs, CP_SET_MODE, 1);
   tu_cs_emit(cs, 0x0);

   if (fdm || (TU_DEBUG(FDM) && cmd->state.pass->has_fdm)) {
      unsigned views =
         cmd->state.pass->num_views ? cmd->state.pass->num_views : 1;
      const struct tu_framebuffer *fb = cmd->state.framebuffer;
      struct tu_frag_area raw_areas[views];
      if (fdm) {
         tu_fragment_density_map_sample(fdm,
                                        (x1 + MIN2(x2, fb->width)) / 2,
                                        (y1 + MIN2(y2, fb->height)) / 2,
                                        fb->width, fb->height, views,
                                        raw_areas);
      } else {
         for (unsigned i = 0; i < views; i++)
            raw_areas[i].width = raw_areas[i].height = 1.0f;
      }

      VkExtent2D frag_areas[views];
      for (unsigned i = 0; i < views; i++) {
         float floor_x, floor_y;
         float area = raw_areas[i].width * raw_areas[i].height;
         float frac_x = modff(raw_areas[i].width, &floor_x);
         float frac_y = modff(raw_areas[i].height, &floor_y);
         /* The spec allows rounding up one of the axes as long as the total
          * area is less than or equal to the original area. Take advantage of
          * this to try rounding up the number with the largest fraction.
          */
         if ((frac_x > frac_y ? (floor_x + 1.f) * floor_y :
                                 floor_x * (floor_y + 1.f)) <= area) {
            if (frac_x > frac_y)
               floor_x += 1.f;
            else
               floor_y += 1.f;
         }
         frag_areas[i].width = floor_x;
         frag_areas[i].height = floor_y;

         /* Make sure that the width/height divides the tile width/height so
          * we don't have to do extra awkward clamping of the edges of each
          * bin when resolving. Note that because the tile width is rounded to
          * a multiple of 32 any power of two 32 or less will work.
          *
          * TODO: Try to take advantage of the total area allowance here, too.
          */
         while (tiling->tile0.width % frag_areas[i].width != 0)
            frag_areas[i].width--;
         while (tiling->tile0.height % frag_areas[i].height != 0)
            frag_areas[i].height--;
      }

      /* If at any point we were forced to use the same scaling for all
       * viewports, we need to make sure that any users *not* using shared
       * scaling, including loads/stores, also consistently share the scaling. 
       */
      if (cmd->state.rp.shared_viewport) {
         VkExtent2D frag_area = { UINT32_MAX, UINT32_MAX };
         for (unsigned i = 0; i < views; i++) {
            frag_area.width = MIN2(frag_area.width, frag_areas[i].width);
            frag_area.height = MIN2(frag_area.height, frag_areas[i].height);
         }

         for (unsigned i = 0; i < views; i++)
            frag_areas[i] = frag_area;
      }

      VkRect2D bin = { { x1, y1 }, { x2 - x1, y2 - y1 } };
      util_dynarray_foreach (&cmd->fdm_bin_patchpoints,
                             struct tu_fdm_bin_patchpoint, patch) {
         tu_cs_emit_pkt7(cs, CP_MEM_WRITE, 2 + patch->size);
         tu_cs_emit_qw(cs, patch->iova);
         patch->apply(cs, patch->data, bin, views, frag_areas);
      }

      /* Make the CP wait until the CP_MEM_WRITE's to the command buffers
       * land.
       */
      tu_cs_emit_pkt7(cs, CP_WAIT_MEM_WRITES, 0);
      tu_cs_emit_pkt7(cs, CP_WAIT_FOR_ME, 0);
   }
}

template <chip CHIP>
static void
tu6_emit_sysmem_resolve(struct tu_cmd_buffer *cmd,
                        struct tu_cs *cs,
                        uint32_t layer_mask,
                        uint32_t a,
                        uint32_t gmem_a)
{
   const struct tu_framebuffer *fb = cmd->state.framebuffer;
   const struct tu_image_view *dst = cmd->state.attachments[a];
   const struct tu_image_view *src = cmd->state.attachments[gmem_a];

   tu_resolve_sysmem<CHIP>(cmd, cs, src, dst, layer_mask, fb->layers, &cmd->state.render_area);
}

template <chip CHIP>
static void
tu6_emit_sysmem_resolves(struct tu_cmd_buffer *cmd,
                         struct tu_cs *cs,
                         const struct tu_subpass *subpass)
{
   if (subpass->resolve_attachments) {
      /* From the documentation for vkCmdNextSubpass, section 7.4 "Render Pass
       * Commands":
       *
       *    End-of-subpass multisample resolves are treated as color
       *    attachment writes for the purposes of synchronization.
       *    This applies to resolve operations for both color and
       *    depth/stencil attachments. That is, they are considered to
       *    execute in the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
       *    pipeline stage and their writes are synchronized with
       *    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT. Synchronization between
       *    rendering within a subpass and any resolve operations at the end
       *    of the subpass occurs automatically, without need for explicit
       *    dependencies or pipeline barriers. However, if the resolve
       *    attachment is also used in a different subpass, an explicit
       *    dependency is needed.
       *
       * We use the CP_BLIT path for sysmem resolves, which is really a
       * transfer command, so we have to manually flush similar to the gmem
       * resolve case. However, a flush afterwards isn't needed because of the
       * last sentence and the fact that we're in sysmem mode.
       */
      tu_emit_event_write<CHIP>(cmd, cs, FD_CCU_FLUSH_COLOR);
      if (subpass->resolve_depth_stencil)
         tu_emit_event_write<CHIP>(cmd, cs, FD_CCU_FLUSH_DEPTH);

      tu_emit_event_write<CHIP>(cmd, cs, FD_CACHE_INVALIDATE);

      /* Wait for the flushes to land before using the 2D engine */
      tu_cs_emit_wfi(cs);

      for (unsigned i = 0; i < subpass->resolve_count; i++) {
         uint32_t a = subpass->resolve_attachments[i].attachment;
         if (a == VK_ATTACHMENT_UNUSED)
            continue;

         uint32_t gmem_a = tu_subpass_get_attachment_to_resolve(subpass, i);

         tu6_emit_sysmem_resolve<CHIP>(cmd, cs, subpass->multiview_mask, a, gmem_a);
      }
   }
}

template <chip CHIP>
static void
tu6_emit_tile_store(struct tu_cmd_buffer *cmd, struct tu_cs *cs)
{
   const struct tu_render_pass *pass = cmd->state.pass;
   const struct tu_subpass *subpass = &pass->subpasses[pass->subpass_count-1];
   const struct tu_framebuffer *fb = cmd->state.framebuffer;

   if (pass->has_fdm)
      tu_cs_set_writeable(cs, true);

   tu_cs_emit_pkt7(cs, CP_SET_MARKER, 1);
   tu_cs_emit(cs, A6XX_CP_SET_MARKER_0_MODE(RM6_RESOLVE));

   tu6_emit_blit_scissor(cmd, cs, true);

   for (uint32_t a = 0; a < pass->attachment_count; ++a) {
      if (pass->attachments[a].gmem) {
         const bool cond_exec_allowed = cmd->state.tiling->binning_possible &&
                                        cmd->state.pass->has_cond_load_store;
         tu_store_gmem_attachment<CHIP>(cmd, cs, a, a,
                                  fb->layers, subpass->multiview_mask,
                                  cond_exec_allowed);
      }
   }

   if (subpass->resolve_attachments) {
      for (unsigned i = 0; i < subpass->resolve_count; i++) {
         uint32_t a = subpass->resolve_attachments[i].attachment;
         if (a != VK_ATTACHMENT_UNUSED) {
            uint32_t gmem_a = tu_subpass_get_attachment_to_resolve(subpass, i);
            tu_store_gmem_attachment<CHIP>(cmd, cs, a, gmem_a, fb->layers,
                                     subpass->multiview_mask, false);
         }
      }
   }

   if (pass->has_fdm)
      tu_cs_set_writeable(cs, false);
}

void
tu_disable_draw_states(struct tu_cmd_buffer *cmd, struct tu_cs *cs)
{
   tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 3);
   tu_cs_emit(cs, CP_SET_DRAW_STATE__0_COUNT(0) |
                     CP_SET_DRAW_STATE__0_DISABLE_ALL_GROUPS |
                     CP_SET_DRAW_STATE__0_GROUP_ID(0));
   tu_cs_emit(cs, CP_SET_DRAW_STATE__1_ADDR_LO(0));
   tu_cs_emit(cs, CP_SET_DRAW_STATE__2_ADDR_HI(0));

   cmd->state.dirty |= TU_CMD_DIRTY_DRAW_STATE;
}

template <chip CHIP>
static void
tu6_init_hw(struct tu_cmd_buffer *cmd, struct tu_cs *cs)
{
   struct tu_device *dev = cmd->device;
   const struct tu_physical_device *phys_dev = dev->physical_device;

   if (CHIP == A6XX) {
      tu_emit_event_write<CHIP>(cmd, cs, FD_CACHE_INVALIDATE);
   } else {
      tu_cs_emit_pkt7(cs, CP_THREAD_CONTROL, 1);
      tu_cs_emit(cs, CP_THREAD_CONTROL_0_THREAD(CP_SET_THREAD_BR));

      tu_emit_event_write<CHIP>(cmd, cs, FD_CCU_INVALIDATE_COLOR);
      tu_emit_event_write<CHIP>(cmd, cs, FD_CCU_INVALIDATE_DEPTH);
      tu_emit_raw_event_write<CHIP>(cmd, cs, UNK_40, false);
      tu_emit_event_write<CHIP>(cmd, cs, FD_CACHE_INVALIDATE);
      tu_cs_emit_wfi(cs);
   }

   tu_cs_emit_regs(cs, HLSQ_INVALIDATE_CMD(CHIP,
         .vs_state = true,
         .hs_state = true,
         .ds_state = true,
         .gs_state = true,
         .fs_state = true,
         .cs_state = true,
         .cs_ibo = true,
         .gfx_ibo = true,
         .cs_shared_const = true,
         .gfx_shared_const = true,
         .cs_bindless = CHIP == A6XX ? 0x1f : 0xff,
         .gfx_bindless = CHIP == A6XX ? 0x1f : 0xff,));

   tu_cs_emit_wfi(cs);

   if (dev->dbg_cmdbuf_stomp_cs) {
      tu_cs_emit_call(cs, dev->dbg_cmdbuf_stomp_cs);
   }

   cmd->state.cache.pending_flush_bits &=
      ~(TU_CMD_FLAG_WAIT_FOR_IDLE | TU_CMD_FLAG_CACHE_INVALIDATE);

   tu_cs_emit_regs(cs, rb_ccu_cntl<CHIP>(cmd->device, false));
   cmd->state.ccu_state = TU_CMD_CCU_SYSMEM;

   for (size_t i = 0; i < ARRAY_SIZE(phys_dev->info->a6xx.magic_raw); i++) {
      auto magic_reg = phys_dev->info->a6xx.magic_raw[i];
      if (!magic_reg.reg)
         break;

      tu_cs_emit_write_reg(cs, magic_reg.reg, magic_reg.value);
   }

   tu_cs_emit_write_reg(cs, REG_A6XX_RB_DBG_ECO_CNTL,
                        phys_dev->info->a6xx.magic.RB_DBG_ECO_CNTL);
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_FLOAT_CNTL, 0);
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_DBG_ECO_CNTL,
                        phys_dev->info->a6xx.magic.SP_DBG_ECO_CNTL);
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_PERFCTR_ENABLE, 0x3f);
   if (CHIP == A6XX)
      tu_cs_emit_write_reg(cs, REG_A6XX_TPL1_UNKNOWN_B605, 0x44);
   tu_cs_emit_write_reg(cs, REG_A6XX_TPL1_DBG_ECO_CNTL,
                        phys_dev->info->a6xx.magic.TPL1_DBG_ECO_CNTL);
   if (CHIP == A6XX) {
      tu_cs_emit_write_reg(cs, REG_A6XX_HLSQ_UNKNOWN_BE00, 0x80);
      tu_cs_emit_write_reg(cs, REG_A6XX_HLSQ_UNKNOWN_BE01, 0);
   }

   tu_cs_emit_write_reg(cs, REG_A6XX_VPC_DBG_ECO_CNTL,
                        phys_dev->info->a6xx.magic.VPC_DBG_ECO_CNTL);
   tu_cs_emit_write_reg(cs, REG_A6XX_GRAS_DBG_ECO_CNTL,
                        phys_dev->info->a6xx.magic.GRAS_DBG_ECO_CNTL);
   if (CHIP == A6XX) {
      tu_cs_emit_write_reg(cs, REG_A6XX_HLSQ_DBG_ECO_CNTL,
                           phys_dev->info->a6xx.magic.HLSQ_DBG_ECO_CNTL);
   }
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_CHICKEN_BITS,
                        phys_dev->info->a6xx.magic.SP_CHICKEN_BITS);
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_IBO_COUNT, 0); // 2 on a740 ???
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_UNKNOWN_B182, 0);
   if (CHIP == A6XX)
      tu_cs_emit_regs(cs, A6XX_HLSQ_SHARED_CONSTS(.enable = false));
   tu_cs_emit_write_reg(cs, REG_A6XX_UCHE_UNKNOWN_0E12,
                        phys_dev->info->a6xx.magic.UCHE_UNKNOWN_0E12);
   tu_cs_emit_write_reg(cs, REG_A6XX_UCHE_CLIENT_PF,
                        phys_dev->info->a6xx.magic.UCHE_CLIENT_PF);
   tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_8E01,
                        phys_dev->info->a6xx.magic.RB_UNKNOWN_8E01);
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_UNKNOWN_A9A8, 0);
   tu_cs_emit_regs(cs, A6XX_SP_MODE_CONTROL(.constant_demotion_enable = true,
                                            .isammode = ISAMMODE_GL,
                                            .shared_consts_enable = false));

   /* TODO: set A6XX_VFD_ADD_OFFSET_INSTANCE and fix ir3 to avoid adding base instance */
   tu_cs_emit_write_reg(cs, REG_A6XX_VFD_ADD_OFFSET, A6XX_VFD_ADD_OFFSET_VERTEX);
   tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_8811, 0x00000010);
   tu_cs_emit_write_reg(cs, REG_A6XX_PC_MODE_CNTL,
                        phys_dev->info->a6xx.magic.PC_MODE_CNTL);

   tu_cs_emit_write_reg(cs, REG_A6XX_GRAS_UNKNOWN_8110, 0);

   tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_8818, 0);

   if (CHIP == A6XX) {
      tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_8819, 0);
      tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_881A, 0);
      tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_881B, 0);
      tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_881C, 0);
      tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_881D, 0);
      tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_881E, 0);
   }

   tu_cs_emit_write_reg(cs, REG_A6XX_RB_UNKNOWN_88F0, 0);

   tu_cs_emit_regs(cs, A6XX_VPC_POINT_COORD_INVERT(false));
   tu_cs_emit_write_reg(cs, REG_A6XX_VPC_UNKNOWN_9300, 0);

   tu_cs_emit_regs(cs, A6XX_VPC_SO_DISABLE(true));

   tu_cs_emit_write_reg(cs, REG_A6XX_SP_UNKNOWN_B183, 0);

   tu_cs_emit_write_reg(cs, REG_A6XX_GRAS_SU_CONSERVATIVE_RAS_CNTL, 0);
   tu_cs_emit_write_reg(cs, REG_A6XX_GRAS_UNKNOWN_80AF, 0);
   if (CHIP == A6XX) {
      tu_cs_emit_write_reg(cs, REG_A6XX_VPC_UNKNOWN_9210, 0);
      tu_cs_emit_write_reg(cs, REG_A6XX_VPC_UNKNOWN_9211, 0);
   }
   tu_cs_emit_write_reg(cs, REG_A6XX_VPC_UNKNOWN_9602, 0);
   tu_cs_emit_write_reg(cs, REG_A6XX_PC_UNKNOWN_9E72, 0);
   tu_cs_emit_write_reg(cs, REG_A6XX_SP_TP_MODE_CNTL,
                        0x000000a0 |
                        A6XX_SP_TP_MODE_CNTL_ISAMMODE(ISAMMODE_GL));
   tu_cs_emit_regs(cs, HLSQ_CONTROL_5_REG(CHIP, .dword = 0xfc));

   tu_cs_emit_write_reg(cs, REG_A6XX_VFD_MODE_CNTL, 0x00000000);

   tu_cs_emit_write_reg(cs, REG_A6XX_PC_MODE_CNTL, phys_dev->info->a6xx.magic.PC_MODE_CNTL);

   tu_cs_emit_regs(cs, A6XX_RB_ALPHA_CONTROL()); /* always disable alpha test */
   tu_cs_emit_regs(cs, A6XX_RB_DITHER_CNTL()); /* always disable dithering */

   tu_disable_draw_states(cmd, cs);

   tu_cs_emit_regs(cs,
                   A6XX_SP_TP_BORDER_COLOR_BASE_ADDR(.bo = dev->global_bo,
                                                     .bo_offset = gb_offset(bcolor_builtin)));
   tu_cs_emit_regs(cs,
                   A6XX_SP_PS_TP_BORDER_COLOR_BASE_ADDR(.bo = dev->global_bo,
                                                        .bo_offset = gb_offset(bcolor_builtin)));

   if (CHIP == A7XX) {
      tu_cs_emit_regs(cs, A6XX_TPL1_BICUBIC_WEIGHTS_TABLE_0(0),
                      A6XX_TPL1_BICUBIC_WEIGHTS_TABLE_1(0x3fe05ff4),
                      A6XX_TPL1_BICUBIC_WEIGHTS_TABLE_2(0x3fa0ebee),
                      A6XX_TPL1_BICUBIC_WEIGHTS_TABLE_3(0x3f5193ed),
                      A6XX_TPL1_BICUBIC_WEIGHTS_TABLE_4(0x3f0243f0), );
   }

   if (phys_dev->info->a7xx.cmdbuf_start_a725_quirk) {
      tu_cs_reserve(cs, 3 + 4);
      tu_cs_emit_pkt7(cs, CP_COND_REG_EXEC, 2);
      tu_cs_emit(cs, CP_COND_REG_EXEC_0_MODE(THREAD_MODE) |
                     CP_COND_REG_EXEC_0_BR | CP_COND_REG_EXEC_0_LPAC);
      tu_cs_emit(cs, RENDER_MODE_CP_COND_REG_EXEC_1_DWORDS(4));
      tu_cs_emit_ib(cs, dev->cmdbuf_start_a725_quirk_entry);
   }

   tu_cs_sanity_check(cs);
}

static void
update_vsc_pipe(struct tu_cmd_buffer *cmd,
                struct tu_cs *cs,
                uint32_t num_vsc_pipes)
{
   const struct tu_tiling_config *tiling = cmd->state.tiling;

   tu_cs_emit_regs(cs,
                   A6XX_VSC_BIN_SIZE(.width = tiling->tile0.width,
                                     .height = tiling->tile0.height));

   tu_cs_emit_regs(cs,
                   A6XX_VSC_BIN_COUNT(.nx = tiling->tile_count.width,
                                      .ny = tiling->tile_count.height));

   tu_cs_emit_pkt4(cs, REG_A6XX_VSC_PIPE_CONFIG_REG(0), num_vsc_pipes);
   tu_cs_emit_array(cs, tiling->pipe_config, num_vsc_pipes);

   tu_cs_emit_regs(cs,
                   A6XX_VSC_PRIM_STRM_PITCH(cmd->vsc_prim_strm_pitch),
                   A6XX_VSC_PRIM_STRM_LIMIT(cmd->vsc_prim_strm_pitch - VSC_PAD));

   tu_cs_emit_regs(cs,
                   A6XX_VSC_DRAW_STRM_PITCH(cmd->vsc_draw_strm_pitch),
                   A6XX_VSC_DRAW_STRM_LIMIT(cmd->vsc_draw_strm_pitch - VSC_PAD));
}

static void
emit_vsc_overflow_test(struct tu_cmd_buffer *cmd, struct tu_cs *cs)
{
   const struct tu_tiling_config *tiling = cmd->state.tiling;
   const uint32_t used_pipe_count =
      tiling->pipe_count.width * tiling->pipe_count.height;

   for (int i = 0; i < used_pipe_count; i++) {
      tu_cs_emit_pkt7(cs, CP_COND_WRITE5, 8);
      tu_cs_emit(cs, CP_COND_WRITE5_0_FUNCTION(WRITE_GE) |
            CP_COND_WRITE5_0_WRITE_MEMORY);
      tu_cs_emit(cs, CP_COND_WRITE5_1_POLL_ADDR_LO(REG_A6XX_VSC_DRAW_STRM_SIZE_REG(i)));
      tu_cs_emit(cs, CP_COND_WRITE5_2_POLL_ADDR_HI(0));
      tu_cs_emit(cs, CP_COND_WRITE5_3_REF(cmd->vsc_draw_strm_pitch - VSC_PAD));
      tu_cs_emit(cs, CP_COND_WRITE5_4_MASK(~0));
      tu_cs_emit_qw(cs, global_iova(cmd, vsc_draw_overflow));
      tu_cs_emit(cs, CP_COND_WRITE5_7_WRITE_DATA(cmd->vsc_draw_strm_pitch));

      tu_cs_emit_pkt7(cs, CP_COND_WRITE5, 8);
      tu_cs_emit(cs, CP_COND_WRITE5_0_FUNCTION(WRITE_GE) |
            CP_COND_WRITE5_0_WRITE_MEMORY);
      tu_cs_emit(cs, CP_COND_WRITE5_1_POLL_ADDR_LO(REG_A6XX_VSC_PRIM_STRM_SIZE_REG(i)));
      tu_cs_emit(cs, CP_COND_WRITE5_2_POLL_ADDR_HI(0));
      tu_cs_emit(cs, CP_COND_WRITE5_3_REF(cmd->vsc_prim_strm_pitch - VSC_PAD));
      tu_cs_emit(cs, CP_COND_WRITE5_4_MASK(~0));
      tu_cs_emit_qw(cs, global_iova(cmd, vsc_prim_overflow));
      tu_cs_emit(cs, CP_COND_WRITE5_7_WRITE_DATA(cmd->vsc_prim_strm_pitch));
   }

   tu_cs_emit_pkt7(cs, CP_WAIT_MEM_WRITES, 0);
}

template <chip CHIP>
static void
tu6_emit_binning_pass(struct tu_cmd_buffer *cmd, struct tu_cs *cs)
{
   struct tu_physical_device *phys_dev = cmd->device->physical_device;
   const struct tu_framebuffer *fb = cmd->state.framebuffer;

   /* If this command buffer may be executed multiple times, then
    * viewports/scissor states may have been changed by previous executions
    * and we need to reset them before executing the binning IB.
    */
   if (!(cmd->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) &&
       cmd->fdm_bin_patchpoints.size != 0) {
      unsigned num_views = MAX2(cmd->state.pass->num_views, 1);
      VkExtent2D unscaled_frag_areas[num_views];
      for (unsigned i = 0; i < num_views; i++)
         unscaled_frag_areas[i] = (VkExtent2D) { 1, 1 };
      VkRect2D bin = { { 0, 0 }, { fb->width, fb->height } };
      util_dynarray_foreach (&cmd->fdm_bin_patchpoints,
                             struct tu_fdm_bin_patchpoint, patch) {
         tu_cs_emit_pkt7(cs, CP_MEM_WRITE, 2 + patch->size);
         tu_cs_emit_qw(cs, patch->iova);
         patch->apply(cs, patch->data, bin, num_views, unscaled_frag_areas);
      }

      tu_cs_emit_pkt7(cs, CP_WAIT_MEM_WRITES, 0);
      tu_cs_emit_pkt7(cs, CP_WAIT_FOR_ME, 0);
   }

   tu6_emit_window_scissor(cs, 0, 0, fb->width - 1, fb->height - 1);

   tu_cs_emit_pkt7(cs, CP_SET_MARKER, 1);
   tu_cs_emit(cs, A6XX_CP_SET_MARKER_0_MODE(RM6_BINNING));

   tu_cs_emit_pkt7(cs, CP_SET_VISIBILITY_OVERRIDE, 1);
   tu_cs_emit(cs, 0x1);

   tu_cs_emit_pkt7(cs, CP_SET_MODE, 1);
   tu_cs_emit(cs, 0x1);

   tu_cs_emit_wfi(cs);

   tu_cs_emit_regs(cs,
                   A6XX_VFD_MODE_CNTL(.render_mode = BINNING_PASS));

   update_vsc_pipe(cmd, cs, phys_dev->info->num_vsc_pipes);

   tu_cs_emit_regs(cs,
                   A6XX_PC_POWER_CNTL(phys_dev->info->a6xx.magic.PC_POWER_CNTL));

   tu_cs_emit_regs(cs,
                   A6XX_VFD_POWER_CNTL(phys_dev->info->a6xx.magic.PC_POWER_CNTL));

   tu_cs_emit_pkt7(cs, CP_EVENT_WRITE, 1);
   tu_cs_emit(cs, UNK_2C);

   tu_cs_emit_regs(cs,
                   A6XX_RB_WINDOW_OFFSET(.x = 0, .y = 0));

   tu_cs_emit_regs(cs,
                   A6XX_SP_TP_WINDOW_OFFSET(.x = 0, .y = 0));

   trace_start_binning_ib(&cmd->trace, cs);

   /* emit IB to binning drawcmds: */
   tu_cs_emit_call(cs, &cmd->draw_cs);

   trace_end_binning_ib(&cmd->trace, cs);

   /* switching from binning pass to GMEM pass will cause a switch from
    * PROGRAM_BINNING to PROGRAM, which invalidates const state (XS_CONST states)
    * so make sure these states are re-emitted
    * (eventually these states shouldn't exist at all with shader prologue)
    * only VS and GS are invalidated, as FS isn't emitted in binning pass,
    * and we don't use HW binning when tesselation is used
    */
   tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 3);
   tu_cs_emit(cs, CP_SET_DRAW_STATE__0_COUNT(0) |
                  CP_SET_DRAW_STATE__0_DISABLE |
                  CP_SET_DRAW_STATE__0_GROUP_ID(TU_DRAW_STATE_CONST));
   tu_cs_emit(cs, CP_SET_DRAW_STATE__1_ADDR_LO(0));
   tu_cs_emit(cs, CP_SET_DRAW_STATE__2_ADDR_HI(0));

   tu_cs_emit_pkt7(cs, CP_EVENT_WRITE, 1);
   tu_cs_emit(cs, UNK_2D);

   /* This flush is probably required because the VSC, which produces the
    * visibility stream, is a client of UCHE, whereas the CP needs to read the
    * visibility stream (without caching) to do draw skipping. The
    * WFI+WAIT_FOR_ME combination guarantees that the binning commands
    * submitted are finished before reading the VSC regs (in
    * emit_vsc_overflow_test) or the VSC_DATA buffer directly (implicitly as
    * part of draws).
    */
   tu_emit_event_write<CHIP>(cmd, cs, FD_CACHE_FLUSH);

   tu_cs_emit_wfi(cs);

   tu_cs_emit_pkt7(cs, CP_WAIT_FOR_ME, 0);

   emit_vsc_overflow_test(cmd, cs);

   tu_cs_emit_pkt7(cs, CP_SET_VISIBILITY_OVERRIDE, 1);
   tu_cs_emit(cs, 0x0);

   tu_cs_emit_pkt7(cs, CP_SET_MODE, 1);
   tu_cs_emit(cs, 0x0);
}

static struct tu_draw_state
tu_emit_input_attachments(struct tu_cmd_buffer *cmd,
                          const struct tu_subpass *subpass,
                          bool gmem)
{
   const struct tu_tiling_config *tiling = cmd->state.tiling;

   /* note: we can probably emit input attachments just once for the whole
    * renderpass, this would avoid emitting both sysmem/gmem versions
    *
    * emit two texture descriptors for each input, as a workaround for
    * d24s8/d32s8, which can be sampled as both float (depth) and integer (stencil)
    * tu_shader lowers uint input attachment loads to use the 2nd descriptor
    * in the pair
    * TODO: a smarter workaround
    */

   if (!subpass->input_count)
      return (struct tu_draw_state) {};

   struct tu_cs_memory texture;
   VkResult result = tu_cs_alloc(&cmd->sub_cs, subpass->input_count * 2,
                                 A6XX_TEX_CONST_DWORDS, &texture);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return (struct tu_draw_state) {};
   }

   for (unsigned i = 0; i < subpass->input_count * 2; i++) {
      uint32_t a = subpass->input_attachments[i / 2].attachment;
      if (a == VK_ATTACHMENT_UNUSED)
         continue;

      const struct tu_image_view *iview = cmd->state.attachments[a];
      const struct tu_render_pass_attachment *att =
         &cmd->state.pass->attachments[a];
      uint32_t *dst = &texture.map[A6XX_TEX_CONST_DWORDS * i];
      uint32_t gmem_offset = tu_attachment_gmem_offset(cmd, att, 0);
      uint32_t cpp = att->cpp;

      memcpy(dst, iview->view.descriptor, A6XX_TEX_CONST_DWORDS * 4);

      /* Cube descriptors require a different sampling instruction in shader,
       * however we don't know whether image is a cube or not until the start
       * of a renderpass. We have to patch the descriptor to make it compatible
       * with how it is sampled in shader.
       */
      enum a6xx_tex_type tex_type =
         (enum a6xx_tex_type)((dst[2] & A6XX_TEX_CONST_2_TYPE__MASK) >>
                              A6XX_TEX_CONST_2_TYPE__SHIFT);
      if (tex_type == A6XX_TEX_CUBE) {
         dst[2] &= ~A6XX_TEX_CONST_2_TYPE__MASK;
         dst[2] |= A6XX_TEX_CONST_2_TYPE(A6XX_TEX_2D);

         uint32_t depth = (dst[5] & A6XX_TEX_CONST_5_DEPTH__MASK) >>
                          A6XX_TEX_CONST_5_DEPTH__SHIFT;
         dst[5] &= ~A6XX_TEX_CONST_5_DEPTH__MASK;
         dst[5] |= A6XX_TEX_CONST_5_DEPTH(depth * 6);
      }

      if (i % 2 == 1 && att->format == VK_FORMAT_D24_UNORM_S8_UINT) {
         /* note this works because spec says fb and input attachments
          * must use identity swizzle
          *
          * Also we clear swap to WZYX.  This is because the view might have
          * picked XYZW to work better with border colors.
          */
         dst[0] &= ~(A6XX_TEX_CONST_0_FMT__MASK |
            A6XX_TEX_CONST_0_SWAP__MASK |
            A6XX_TEX_CONST_0_SWIZ_X__MASK | A6XX_TEX_CONST_0_SWIZ_Y__MASK |
            A6XX_TEX_CONST_0_SWIZ_Z__MASK | A6XX_TEX_CONST_0_SWIZ_W__MASK);
         if (!cmd->device->physical_device->info->a6xx.has_z24uint_s8uint) {
            dst[0] |= A6XX_TEX_CONST_0_FMT(FMT6_8_8_8_8_UINT) |
               A6XX_TEX_CONST_0_SWIZ_X(A6XX_TEX_W) |
               A6XX_TEX_CONST_0_SWIZ_Y(A6XX_TEX_ZERO) |
               A6XX_TEX_CONST_0_SWIZ_Z(A6XX_TEX_ZERO) |
               A6XX_TEX_CONST_0_SWIZ_W(A6XX_TEX_ONE);
         } else {
            dst[0] |= A6XX_TEX_CONST_0_FMT(FMT6_Z24_UINT_S8_UINT) |
               A6XX_TEX_CONST_0_SWIZ_X(A6XX_TEX_Y) |
               A6XX_TEX_CONST_0_SWIZ_Y(A6XX_TEX_ZERO) |
               A6XX_TEX_CONST_0_SWIZ_Z(A6XX_TEX_ZERO) |
               A6XX_TEX_CONST_0_SWIZ_W(A6XX_TEX_ONE);
         }
      }

      if (i % 2 == 1 && att->format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
         dst[0] &= ~A6XX_TEX_CONST_0_FMT__MASK;
         dst[0] |= A6XX_TEX_CONST_0_FMT(FMT6_8_UINT);
         dst[2] &= ~(A6XX_TEX_CONST_2_PITCHALIGN__MASK | A6XX_TEX_CONST_2_PITCH__MASK);
         dst[2] |= A6XX_TEX_CONST_2_PITCH(iview->stencil_pitch);
         dst[3] = 0;
         dst[4] = iview->stencil_base_addr;
         dst[5] = (dst[5] & 0xffff) | iview->stencil_base_addr >> 32;

         cpp = att->samples;
         gmem_offset = att->gmem_offset_stencil[cmd->state.gmem_layout];
      }

      if (!gmem || !subpass->input_attachments[i / 2].patch_input_gmem)
         continue;

      /* patched for gmem */
      dst[0] &= ~(A6XX_TEX_CONST_0_SWAP__MASK | A6XX_TEX_CONST_0_TILE_MODE__MASK);
      dst[0] |= A6XX_TEX_CONST_0_TILE_MODE(TILE6_2);
      dst[2] =
         A6XX_TEX_CONST_2_TYPE(A6XX_TEX_2D) |
         A6XX_TEX_CONST_2_PITCH(tiling->tile0.width * cpp);
      /* Note: it seems the HW implicitly calculates the array pitch with the
       * GMEM tiling, so we don't need to specify the pitch ourselves.
       */
      dst[3] = 0;
      dst[4] = cmd->device->physical_device->gmem_base + gmem_offset;
      dst[5] &= A6XX_TEX_CONST_5_DEPTH__MASK;
      for (unsigned i = 6; i < A6XX_TEX_CONST_DWORDS; i++)
         dst[i] = 0;
   }

   struct tu_cs cs;
   struct tu_draw_state ds = tu_cs_draw_state(&cmd->sub_cs, &cs, 9);

   tu_cs_emit_pkt7(&cs, CP_LOAD_STATE6_FRAG, 3);
   tu_cs_emit(&cs, CP_LOAD_STATE6_0_DST_OFF(0) |
                  CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                  CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                  CP_LOAD_STATE6_0_STATE_BLOCK(SB6_FS_TEX) |
                  CP_LOAD_STATE6_0_NUM_UNIT(subpass->input_count * 2));
   tu_cs_emit_qw(&cs, texture.iova);

   tu_cs_emit_regs(&cs, A6XX_SP_FS_TEX_CONST(.qword = texture.iova));

   tu_cs_emit_regs(&cs, A6XX_SP_FS_TEX_COUNT(subpass->input_count * 2));

   assert(cs.cur == cs.end); /* validate draw state size */

   return ds;
}

static void
tu_set_input_attachments(struct tu_cmd_buffer *cmd, const struct tu_subpass *subpass)
{
   struct tu_cs *cs = &cmd->draw_cs;

   tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 6);
   tu_cs_emit_draw_state(cs, TU_DRAW_STATE_INPUT_ATTACHMENTS_GMEM,
                         tu_emit_input_attachments(cmd, subpass, true));
   tu_cs_emit_draw_state(cs, TU_DRAW_STATE_INPUT_ATTACHMENTS_SYSMEM,
                         tu_emit_input_attachments(cmd, subpass, false));
}

static void
tu_emit_renderpass_begin(struct tu_cmd_buffer *cmd)
{
   /* We need to re-emit any draw states that are patched in order for them to
    * be correctly added to the per-renderpass patchpoint list, even if they
    * are the same as before.
    */
   if (cmd->state.pass->has_fdm)
      cmd->state.dirty |= TU_CMD_DIRTY_FDM;

   /* We need to re-emit MSAA at the beginning of every renderpass because it
    * isn't part of a draw state that gets automatically re-emitted.
    */
   BITSET_SET(cmd->vk.dynamic_graphics_state.dirty,
              MESA_VK_DYNAMIC_MS_RASTERIZATION_SAMPLES);
   /* PC_PRIMITIVE_CNTL_0 isn't a part of a draw state and may be changed
    * by blits.
    */
   BITSET_SET(cmd->vk.dynamic_graphics_state.dirty,
              MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE);
}

template <chip CHIP>
static void
tu6_sysmem_render_begin(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                        struct tu_renderpass_result *autotune_result)
{
   const struct tu_framebuffer *fb = cmd->state.framebuffer;

   tu_lrz_sysmem_begin(cmd, cs);

   assert(fb->width > 0 && fb->height > 0);
   tu6_emit_window_scissor(cs, 0, 0, fb->width - 1, fb->height - 1);
   tu6_emit_window_offset<CHIP>(cs, 0, 0);

   tu6_emit_bin_size<CHIP>(cs, 0, 0, {
      .render_mode = RENDERING_PASS,
      .force_lrz_write_dis = true,
      .buffers_location = BUFFERS_IN_SYSMEM,
      .lrz_feedback_zmode_mask = 0x0,
   });

   if (CHIP == A7XX) {
      tu_cs_emit_regs(cs,
                     A7XX_RB_UNKNOWN_8812(0x3ff)); // all buffers in sysmem
      tu_cs_emit_regs(cs,
                   A7XX_RB_UNKNOWN_88E5(0x50120004));
      tu_cs_emit_regs(cs,
                   A7XX_RB_UNKNOWN_8E06(0x2080000));

      /* These three have something to do with lrz/depth */
      tu_cs_emit_regs(cs, A7XX_GRAS_UNKNOWN_8007(0x0));
      tu_cs_emit_regs(cs, A7XX_GRAS_UNKNOWN_810B(0x3));
      tu_cs_emit_regs(cs, A7XX_GRAS_UNKNOWN_8113(0x4));

      tu_cs_emit_regs(cs, A6XX_GRAS_UNKNOWN_8110(0x2));
      tu_cs_emit_regs(cs, A7XX_RB_UNKNOWN_8E09(0x4));
   }

   tu_cs_emit_pkt7(cs, CP_SET_MARKER, 1);
   tu_cs_emit(cs, A6XX_CP_SET_MARKER_0_MODE(RM6_BYPASS));

   /* A7XX TODO: blob doesn't use CP_SKIP_IB2_ENABLE_* */
   tu_cs_emit_pkt7(cs, CP_SKIP_IB2_ENABLE_GLOBAL, 1);
   tu_cs_emit(cs, 0x0);

   tu_emit_cache_flush_ccu<CHIP>(cmd, cs, TU_CMD_CCU_SYSMEM);

   tu_cs_emit_pkt7(cs, CP_SET_VISIBILITY_OVERRIDE, 1);
   tu_cs_emit(cs, 0x1);

   tu_cs_emit_pkt7(cs, CP_SET_MODE, 1);
   tu_cs_emit(cs, 0x0);

   tu_autotune_begin_renderpass(cmd, cs, autotune_result);

   tu_cs_sanity_check(cs);
}

template <chip CHIP>
static void
tu6_sysmem_render_end(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                      struct tu_renderpass_result *autotune_result)
{
   tu_autotune_end_renderpass(cmd, cs, autotune_result);

   /* Do any resolves of the last subpass. These are handled in the
    * tile_store_cs in the gmem path.
    */
   tu6_emit_sysmem_resolves<CHIP>(cmd, cs, cmd->state.subpass);

   tu_cs_emit_call(cs, &cmd->draw_epilogue_cs);

   tu_cs_emit_pkt7(cs, CP_SKIP_IB2_ENABLE_GLOBAL, 1);
   tu_cs_emit(cs, 0x0);

   tu_lrz_sysmem_end(cmd, cs);

   tu_cs_sanity_check(cs);
}

template <chip CHIP>
static void
tu6_tile_render_begin(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                      struct tu_renderpass_result *autotune_result)
{
   struct tu_physical_device *phys_dev = cmd->device->physical_device;
   const struct tu_tiling_config *tiling = cmd->state.tiling;
   tu_lrz_tiling_begin(cmd, cs);

   tu_cs_emit_pkt7(cs, CP_SKIP_IB2_ENABLE_GLOBAL, 1);
   tu_cs_emit(cs, 0x0);

   tu_emit_cache_flush_ccu<CHIP>(cmd, cs, TU_CMD_CCU_GMEM);

   if (use_hw_binning(cmd)) {
      if (!cmd->vsc_initialized) {
         tu6_lazy_emit_vsc(cmd, cs);
      }

      tu6_emit_bin_size<CHIP>(cs, tiling->tile0.width, tiling->tile0.height,
                              {
                                 .render_mode = BINNING_PASS,
                                 .buffers_location = BUFFERS_IN_GMEM,
                                 .lrz_feedback_zmode_mask = 0x6,
                              });

      tu6_emit_render_cntl<CHIP>(cmd, cmd->state.subpass, cs, true);

      tu6_emit_binning_pass<CHIP>(cmd, cs);

      tu6_emit_bin_size<CHIP>(cs, tiling->tile0.width, tiling->tile0.height,
                              {
                                 .render_mode = RENDERING_PASS,
                                 .force_lrz_write_dis = true,
                                 .buffers_location = BUFFERS_IN_GMEM,
                                 .lrz_feedback_zmode_mask = 0x6,
                              });

      tu_cs_emit_regs(cs,
                      A6XX_VFD_MODE_CNTL(RENDERING_PASS));

      tu_cs_emit_regs(cs,
                      A6XX_PC_POWER_CNTL(phys_dev->info->a6xx.magic.PC_POWER_CNTL));

      tu_cs_emit_regs(cs,
                      A6XX_VFD_POWER_CNTL(phys_dev->info->a6xx.magic.PC_POWER_CNTL));

      tu_cs_emit_pkt7(cs, CP_SKIP_IB2_ENABLE_GLOBAL, 1);
      tu_cs_emit(cs, 0x1);
      tu_cs_emit_pkt7(cs, CP_SKIP_IB2_ENABLE_LOCAL, 1);
      tu_cs_emit(cs, 0x1);
   } else {
      tu6_emit_bin_size<CHIP>(cs, tiling->tile0.width, tiling->tile0.height,
                              {
                                 .render_mode = RENDERING_PASS,
                                 .buffers_location = BUFFERS_IN_GMEM,
                                 .lrz_feedback_zmode_mask = 0x6,
                              });

      if (tiling->binning_possible) {
         /* Mark all tiles as visible for tu6_emit_cond_for_load_stores(), since
          * the actual binner didn't run.
          */
         int pipe_count = tiling->pipe_count.width * tiling->pipe_count.height;
         tu_cs_emit_pkt4(cs, REG_A6XX_VSC_STATE_REG(0), pipe_count);
         for (int i = 0; i < pipe_count; i++)
            tu_cs_emit(cs, ~0);
      }
   }

   tu_autotune_begin_renderpass(cmd, cs, autotune_result);

   tu_cs_sanity_check(cs);
}

template <chip CHIP>
static void
tu6_render_tile(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                uint32_t tx, uint32_t ty, uint32_t pipe, uint32_t slot,
                const struct tu_image_view *fdm)
{
   tu6_emit_tile_select<CHIP>(cmd, &cmd->cs, tx, ty, pipe, slot, fdm);

   trace_start_draw_ib_gmem(&cmd->trace, &cmd->cs);

   /* Primitives that passed all tests are still counted in in each
    * tile even with HW binning beforehand. Do not permit it.
    */
   if (cmd->state.prim_generated_query_running_before_rp)
      tu_emit_event_write<CHIP>(cmd, cs, FD_STOP_PRIMITIVE_CTRS);

   tu_cs_emit_call(cs, &cmd->draw_cs);

   if (cmd->state.prim_generated_query_running_before_rp)
      tu_emit_event_write<CHIP>(cmd, cs, FD_START_PRIMITIVE_CTRS);

   if (use_hw_binning(cmd)) {
      tu_cs_emit_pkt7(cs, CP_SET_MARKER, 1);
      tu_cs_emit(cs, A6XX_CP_SET_MARKER_0_MODE(RM6_ENDVIS));
   }

   /* Predicate is changed in draw_cs so we have to re-emit it */
   if (cmd->state.rp.draw_cs_writes_to_cond_pred)
      tu6_emit_cond_for_load_stores(cmd, cs, pipe, slot, false);

   tu_cs_emit_pkt7(cs, CP_SKIP_IB2_ENABLE_GLOBAL, 1);
   tu_cs_emit(cs, 0x0);

   tu_cs_emit_call(cs, &cmd->tile_store_cs);

   tu_clone_trace_range(cmd, cs, cmd->trace_renderpass_start,
         cmd->trace_renderpass_end);

   tu_cs_sanity_check(cs);

   trace_end_draw_ib_gmem(&cmd->trace, &cmd->cs);
}

template <chip CHIP>
static void
tu6_tile_render_end(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                    struct tu_renderpass_result *autotune_result)
{
   tu_autotune_end_renderpass(cmd, cs, autotune_result);

   tu_cs_emit_call(cs, &cmd->draw_epilogue_cs);

   tu_lrz_tiling_end(cmd, cs);

   tu_emit_event_write<CHIP>(cmd, cs, FD_CCU_FLUSH_BLIT_CACHE);

   tu_cs_sanity_check(cs);
}

template <chip CHIP>
static void
tu_cmd_render_tiles(struct tu_cmd_buffer *cmd,
                    struct tu_renderpass_result *autotune_result)
{
   const struct tu_tiling_config *tiling = cmd->state.tiling;
   const struct tu_image_view *fdm = NULL;

   if (cmd->state.pass->fragment_density_map.attachment != VK_ATTACHMENT_UNUSED) {
      fdm = cmd->state.attachments[cmd->state.pass->fragment_density_map.attachment];
   }

   /* Create gmem stores now (at EndRenderPass time)) because they needed to
    * know whether to allow their conditional execution, which was tied to a
    * state that was known only at the end of the renderpass.  They will be
    * called from tu6_render_tile().
    */
   tu_cs_begin(&cmd->tile_store_cs);
   tu6_emit_tile_store<CHIP>(cmd, &cmd->tile_store_cs);
   tu_cs_end(&cmd->tile_store_cs);

   cmd->trace_renderpass_end = u_trace_end_iterator(&cmd->trace);

   tu6_tile_render_begin<CHIP>(cmd, &cmd->cs, autotune_result);

   /* Note: we reverse the order of walking the pipes and tiles on every
    * other row, to improve texture cache locality compared to raster order.
    */
   for (uint32_t py = 0; py < tiling->pipe_count.height; py++) {
      uint32_t pipe_row = py * tiling->pipe_count.width;
      for (uint32_t pipe_row_i = 0; pipe_row_i < tiling->pipe_count.width; pipe_row_i++) {
         uint32_t px;
         if (py & 1)
            px = tiling->pipe_count.width - 1 - pipe_row_i;
         else
            px = pipe_row_i;
         uint32_t pipe = pipe_row + px;
         uint32_t tx1 = px * tiling->pipe0.width;
         uint32_t ty1 = py * tiling->pipe0.height;
         uint32_t tx2 = MIN2(tx1 + tiling->pipe0.width, tiling->tile_count.width);
         uint32_t ty2 = MIN2(ty1 + tiling->pipe0.height, tiling->tile_count.height);
         uint32_t tile_row_stride = tx2 - tx1;
         uint32_t slot_row = 0;
         for (uint32_t ty = ty1; ty < ty2; ty++) {
            for (uint32_t tile_row_i = 0; tile_row_i < tile_row_stride; tile_row_i++) {
               uint32_t tx;
               if (ty & 1)
                  tx = tile_row_stride - 1 - tile_row_i;
               else
                  tx = tile_row_i;
               uint32_t slot = slot_row + tx;
               tu6_render_tile<CHIP>(cmd, &cmd->cs, tx1 + tx, ty, pipe, slot, fdm);
            }
            slot_row += tile_row_stride;
         }
      }
   }

   tu6_tile_render_end<CHIP>(cmd, &cmd->cs, autotune_result);

   trace_end_render_pass(&cmd->trace, &cmd->cs);

   /* We have trashed the dynamically-emitted viewport, scissor, and FS params
    * via the patchpoints, so we need to re-emit them if they are reused for a
    * later render pass.
    */
   if (cmd->state.pass->has_fdm)
      cmd->state.dirty |= TU_CMD_DIRTY_FDM;

   /* tu6_render_tile has cloned these tracepoints for each tile */
   if (!u_trace_iterator_equal(cmd->trace_renderpass_start, cmd->trace_renderpass_end))
      u_trace_disable_event_range(cmd->trace_renderpass_start,
                                  cmd->trace_renderpass_end);

   /* Reset the gmem store CS entry lists so that the next render pass
    * does its own stores.
    */
   tu_cs_discard_entries(&cmd->tile_store_cs);
}

template <chip CHIP>
static void
tu_cmd_render_sysmem(struct tu_cmd_buffer *cmd,
                     struct tu_renderpass_result *autotune_result)
{
   cmd->trace_renderpass_end = u_trace_end_iterator(&cmd->trace);

   tu6_sysmem_render_begin<CHIP>(cmd, &cmd->cs, autotune_result);

   trace_start_draw_ib_sysmem(&cmd->trace, &cmd->cs);

   tu_cs_emit_call(&cmd->cs, &cmd->draw_cs);

   trace_end_draw_ib_sysmem(&cmd->trace, &cmd->cs);

   tu6_sysmem_render_end<CHIP>(cmd, &cmd->cs, autotune_result);

   trace_end_render_pass(&cmd->trace, &cmd->cs);
}

template <chip CHIP>
void
tu_cmd_render(struct tu_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->state.rp.has_tess)
      tu6_lazy_emit_tessfactor_addr<CHIP>(cmd_buffer);

   struct tu_renderpass_result *autotune_result = NULL;
   if (use_sysmem_rendering(cmd_buffer, &autotune_result))
      tu_cmd_render_sysmem<CHIP>(cmd_buffer, autotune_result);
   else
      tu_cmd_render_tiles<CHIP>(cmd_buffer, autotune_result);

   /* Outside of renderpasses we assume all draw states are disabled. We do
    * this outside the draw CS for the normal case where 3d gmem stores aren't
    * used.
    */
   tu_disable_draw_states(cmd_buffer, &cmd_buffer->cs);

}

static void tu_reset_render_pass(struct tu_cmd_buffer *cmd_buffer)
{
   /* discard draw_cs and draw_epilogue_cs entries now that the tiles are
      rendered */
   tu_cs_discard_entries(&cmd_buffer->draw_cs);
   tu_cs_begin(&cmd_buffer->draw_cs);
   tu_cs_discard_entries(&cmd_buffer->draw_epilogue_cs);
   tu_cs_begin(&cmd_buffer->draw_epilogue_cs);

   cmd_buffer->state.pass = NULL;
   cmd_buffer->state.subpass = NULL;
   cmd_buffer->state.framebuffer = NULL;
   cmd_buffer->state.attachments = NULL;
   cmd_buffer->state.clear_values = NULL;
   cmd_buffer->state.gmem_layout = TU_GMEM_LAYOUT_COUNT; /* invalid value to prevent looking up gmem offsets */
   memset(&cmd_buffer->state.rp, 0, sizeof(cmd_buffer->state.rp));

   /* LRZ is not valid next time we use it */
   cmd_buffer->state.lrz.valid = false;
   cmd_buffer->state.dirty |= TU_CMD_DIRTY_LRZ;

   /* Patchpoints have been executed */
   util_dynarray_clear(&cmd_buffer->fdm_bin_patchpoints);
   ralloc_free(cmd_buffer->patchpoints_ctx);
   cmd_buffer->patchpoints_ctx = NULL;
}

static VkResult
tu_create_cmd_buffer(struct vk_command_pool *pool,
                     struct vk_command_buffer **cmd_buffer_out)
{
   struct tu_device *device =
      container_of(pool->base.device, struct tu_device, vk);
   struct tu_cmd_buffer *cmd_buffer;

   cmd_buffer = (struct tu_cmd_buffer *) vk_zalloc2(
      &device->vk.alloc, NULL, sizeof(*cmd_buffer), 8,
      VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   if (cmd_buffer == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result =
      vk_command_buffer_init(pool, &cmd_buffer->vk, &tu_cmd_buffer_ops,
                             VK_COMMAND_BUFFER_LEVEL_PRIMARY);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, NULL, cmd_buffer);
      return result;
   }

   cmd_buffer->device = device;

   u_trace_init(&cmd_buffer->trace, &device->trace_context);
   list_inithead(&cmd_buffer->renderpass_autotune_results);

   tu_cs_init(&cmd_buffer->cs, device, TU_CS_MODE_GROW, 4096, "cmd cs");
   tu_cs_init(&cmd_buffer->draw_cs, device, TU_CS_MODE_GROW, 4096, "draw cs");
   tu_cs_init(&cmd_buffer->tile_store_cs, device, TU_CS_MODE_GROW, 2048, "tile store cs");
   tu_cs_init(&cmd_buffer->draw_epilogue_cs, device, TU_CS_MODE_GROW, 4096, "draw epilogue cs");
   tu_cs_init(&cmd_buffer->sub_cs, device, TU_CS_MODE_SUB_STREAM, 2048, "draw sub cs");
   tu_cs_init(&cmd_buffer->pre_chain.draw_cs, device, TU_CS_MODE_GROW, 4096, "prechain draw cs");
   tu_cs_init(&cmd_buffer->pre_chain.draw_epilogue_cs, device, TU_CS_MODE_GROW, 4096, "prechain draw epiligoue cs");

   for (unsigned i = 0; i < MAX_BIND_POINTS; i++)
      cmd_buffer->descriptors[i].push_set.base.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;

   *cmd_buffer_out = &cmd_buffer->vk;

   return VK_SUCCESS;
}

static void
tu_cmd_buffer_destroy(struct vk_command_buffer *vk_cmd_buffer)
{
   struct tu_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct tu_cmd_buffer, vk);

   tu_cs_finish(&cmd_buffer->cs);
   tu_cs_finish(&cmd_buffer->draw_cs);
   tu_cs_finish(&cmd_buffer->tile_store_cs);
   tu_cs_finish(&cmd_buffer->draw_epilogue_cs);
   tu_cs_finish(&cmd_buffer->sub_cs);
   tu_cs_finish(&cmd_buffer->pre_chain.draw_cs);
   tu_cs_finish(&cmd_buffer->pre_chain.draw_epilogue_cs);

   u_trace_fini(&cmd_buffer->trace);

   tu_autotune_free_results(cmd_buffer->device, &cmd_buffer->renderpass_autotune_results);

   for (unsigned i = 0; i < MAX_BIND_POINTS; i++) {
      if (cmd_buffer->descriptors[i].push_set.layout)
         vk_descriptor_set_layout_unref(&cmd_buffer->device->vk,
                                        &cmd_buffer->descriptors[i].push_set.layout->vk);
      vk_free(&cmd_buffer->device->vk.alloc,
              cmd_buffer->descriptors[i].push_set.mapped_ptr);
   }

   ralloc_free(cmd_buffer->patchpoints_ctx);
   util_dynarray_fini(&cmd_buffer->fdm_bin_patchpoints);

   vk_command_buffer_finish(&cmd_buffer->vk);
   vk_free2(&cmd_buffer->device->vk.alloc, &cmd_buffer->vk.pool->alloc,
            cmd_buffer);
}

static void
tu_reset_cmd_buffer(struct vk_command_buffer *vk_cmd_buffer,
                    UNUSED VkCommandBufferResetFlags flags)
{
   struct tu_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct tu_cmd_buffer, vk);

   vk_command_buffer_reset(&cmd_buffer->vk);

   tu_cs_reset(&cmd_buffer->cs);
   tu_cs_reset(&cmd_buffer->draw_cs);
   tu_cs_reset(&cmd_buffer->tile_store_cs);
   tu_cs_reset(&cmd_buffer->draw_epilogue_cs);
   tu_cs_reset(&cmd_buffer->sub_cs);
   tu_cs_reset(&cmd_buffer->pre_chain.draw_cs);
   tu_cs_reset(&cmd_buffer->pre_chain.draw_epilogue_cs);

   tu_autotune_free_results(cmd_buffer->device, &cmd_buffer->renderpass_autotune_results);

   for (unsigned i = 0; i < MAX_BIND_POINTS; i++) {
      memset(&cmd_buffer->descriptors[i].sets, 0, sizeof(cmd_buffer->descriptors[i].sets));
      if (cmd_buffer->descriptors[i].push_set.layout) {
         vk_descriptor_set_layout_unref(&cmd_buffer->device->vk,
                                        &cmd_buffer->descriptors[i].push_set.layout->vk);
      }
      memset(&cmd_buffer->descriptors[i].push_set, 0, sizeof(cmd_buffer->descriptors[i].push_set));
      cmd_buffer->descriptors[i].push_set.base.type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
      cmd_buffer->descriptors[i].max_sets_bound = 0;
      cmd_buffer->descriptors[i].max_dynamic_offset_size = 0;
   }

   u_trace_fini(&cmd_buffer->trace);
   u_trace_init(&cmd_buffer->trace, &cmd_buffer->device->trace_context);

   cmd_buffer->state.max_vbs_bound = 0;

   cmd_buffer->vsc_initialized = false;

   ralloc_free(cmd_buffer->patchpoints_ctx);
   cmd_buffer->patchpoints_ctx = NULL;
   util_dynarray_clear(&cmd_buffer->fdm_bin_patchpoints);
}

const struct vk_command_buffer_ops tu_cmd_buffer_ops = {
   .create = tu_create_cmd_buffer,
   .reset = tu_reset_cmd_buffer,
   .destroy = tu_cmd_buffer_destroy,
};

/* Initialize the cache, assuming all necessary flushes have happened but *not*
 * invalidations.
 */
static void
tu_cache_init(struct tu_cache_state *cache)
{
   cache->flush_bits = 0;
   cache->pending_flush_bits = TU_CMD_FLAG_ALL_INVALIDATE;
}

/* Unlike the public entrypoint, this doesn't handle cache tracking, and
 * tracking the CCU state. It's used for the driver to insert its own command
 * buffer in the middle of a submit.
 */
VkResult
tu_cmd_buffer_begin(struct tu_cmd_buffer *cmd_buffer,
                    const VkCommandBufferBeginInfo *pBeginInfo)
{
   vk_command_buffer_begin(&cmd_buffer->vk, pBeginInfo);

   memset(&cmd_buffer->state, 0, sizeof(cmd_buffer->state));
   cmd_buffer->vk.dynamic_graphics_state = vk_default_dynamic_graphics_state;
   cmd_buffer->vk.dynamic_graphics_state.vi = &cmd_buffer->state.vi;
   cmd_buffer->vk.dynamic_graphics_state.ms.sample_locations = &cmd_buffer->state.sl;
   cmd_buffer->state.index_size = 0xff; /* dirty restart index */
   cmd_buffer->state.gmem_layout = TU_GMEM_LAYOUT_COUNT; /* dirty value */

   tu_cache_init(&cmd_buffer->state.cache);
   tu_cache_init(&cmd_buffer->state.renderpass_cache);
   cmd_buffer->usage_flags = pBeginInfo->flags;

   tu_cs_begin(&cmd_buffer->cs);
   tu_cs_begin(&cmd_buffer->draw_cs);
   tu_cs_begin(&cmd_buffer->draw_epilogue_cs);

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_BeginCommandBuffer(VkCommandBuffer commandBuffer,
                      const VkCommandBufferBeginInfo *pBeginInfo)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, commandBuffer);
   VkResult result = tu_cmd_buffer_begin(cmd_buffer, pBeginInfo);
   if (result != VK_SUCCESS)
      return result;

   /* setup initial configuration into command buffer */
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      trace_start_cmd_buffer(&cmd_buffer->trace, &cmd_buffer->cs, cmd_buffer);

      switch (cmd_buffer->queue_family_index) {
      case TU_QUEUE_GENERAL:
         TU_CALLX(cmd_buffer->device, tu6_init_hw)(cmd_buffer, &cmd_buffer->cs);
         break;
      default:
         break;
      }
   } else if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
      const bool pass_continue =
         pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

      trace_start_cmd_buffer(&cmd_buffer->trace,
            pass_continue ? &cmd_buffer->draw_cs : &cmd_buffer->cs, cmd_buffer);

      assert(pBeginInfo->pInheritanceInfo);

      cmd_buffer->inherited_pipeline_statistics =
         pBeginInfo->pInheritanceInfo->pipelineStatistics;

      vk_foreach_struct_const(ext, pBeginInfo->pInheritanceInfo) {
         switch (ext->sType) {
         case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT: {
            const VkCommandBufferInheritanceConditionalRenderingInfoEXT *cond_rend =
               (VkCommandBufferInheritanceConditionalRenderingInfoEXT *) ext;
            cmd_buffer->state.predication_active = cond_rend->conditionalRenderingEnable;
            break;
         }
         default:
            break;
         }
      }

      if (pass_continue) {
         const VkCommandBufferInheritanceRenderingInfo *rendering_info =
            vk_find_struct_const(pBeginInfo->pInheritanceInfo->pNext,
                                 COMMAND_BUFFER_INHERITANCE_RENDERING_INFO);

         if (TU_DEBUG(DYNAMIC)) {
            rendering_info =
               vk_get_command_buffer_inheritance_rendering_info(cmd_buffer->vk.level,
                                                                pBeginInfo);
         }

         if (rendering_info) {
            tu_setup_dynamic_inheritance(cmd_buffer, rendering_info);
            cmd_buffer->state.pass = &cmd_buffer->dynamic_pass;
            cmd_buffer->state.subpass = &cmd_buffer->dynamic_subpass;
         } else {
            cmd_buffer->state.pass = tu_render_pass_from_handle(pBeginInfo->pInheritanceInfo->renderPass);
            cmd_buffer->state.subpass =
               &cmd_buffer->state.pass->subpasses[pBeginInfo->pInheritanceInfo->subpass];
         }
         tu_fill_render_pass_state(&cmd_buffer->state.vk_rp,
                                   cmd_buffer->state.pass,
                                   cmd_buffer->state.subpass);
         vk_cmd_set_cb_attachment_count(&cmd_buffer->vk,
                                        cmd_buffer->state.subpass->color_count);
         cmd_buffer->state.dirty |= TU_CMD_DIRTY_SUBPASS;

         cmd_buffer->patchpoints_ctx = ralloc_parent(NULL);

         /* We can't set the gmem layout here, because the state.pass only has
          * to be compatible (same formats/sample counts) with the primary's
          * renderpass, rather than exactly equal.
          */

         tu_lrz_begin_secondary_cmdbuf(cmd_buffer);
      } else {
         /* When executing in the middle of another command buffer, the CCU
          * state is unknown.
          */
         cmd_buffer->state.ccu_state = TU_CMD_CCU_UNKNOWN;
      }
   }

   return VK_SUCCESS;
}

static struct tu_cs
tu_cmd_dynamic_state(struct tu_cmd_buffer *cmd, uint32_t id, uint32_t size)
{
   struct tu_cs cs;

   assert(id < ARRAY_SIZE(cmd->state.dynamic_state));
   cmd->state.dynamic_state[id] = tu_cs_draw_state(&cmd->sub_cs, &cs, size);

   /* note: this also avoids emitting draw states before renderpass clears,
    * which may use the 3D clear path (for MSAA cases)
    */
   if (cmd->state.dirty & TU_CMD_DIRTY_DRAW_STATE)
      return cs;

   tu_cs_emit_pkt7(&cmd->draw_cs, CP_SET_DRAW_STATE, 3);
   tu_cs_emit_draw_state(&cmd->draw_cs, TU_DRAW_STATE_DYNAMIC + id, cmd->state.dynamic_state[id]);

   return cs;
}

static void
tu_cmd_end_dynamic_state(struct tu_cmd_buffer *cmd, struct tu_cs *cs,
                         uint32_t id)
{
   assert(id < ARRAY_SIZE(cmd->state.dynamic_state));
   cmd->state.dynamic_state[id] = tu_cs_end_draw_state(&cmd->sub_cs, cs);

   /* note: this also avoids emitting draw states before renderpass clears,
    * which may use the 3D clear path (for MSAA cases)
    */
   if (cmd->state.dirty & TU_CMD_DIRTY_DRAW_STATE)
      return;

   tu_cs_emit_pkt7(&cmd->draw_cs, CP_SET_DRAW_STATE, 3);
   tu_cs_emit_draw_state(&cmd->draw_cs, TU_DRAW_STATE_DYNAMIC + id, cmd->state.dynamic_state[id]);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBindVertexBuffers2(VkCommandBuffer commandBuffer,
                         uint32_t firstBinding,
                         uint32_t bindingCount,
                         const VkBuffer *pBuffers,
                         const VkDeviceSize *pOffsets,
                         const VkDeviceSize *pSizes,
                         const VkDeviceSize *pStrides)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs cs;

   cmd->state.max_vbs_bound = MAX2(
      cmd->state.max_vbs_bound, firstBinding + bindingCount);

   if (pStrides) {
      vk_cmd_set_vertex_binding_strides(&cmd->vk, firstBinding, bindingCount,
                                        pStrides);
   }

   cmd->state.vertex_buffers.iova =
      tu_cs_draw_state(&cmd->sub_cs, &cs, 4 * cmd->state.max_vbs_bound).iova;

   for (uint32_t i = 0; i < bindingCount; i++) {
      if (pBuffers[i] == VK_NULL_HANDLE) {
         cmd->state.vb[firstBinding + i].base = 0;
         cmd->state.vb[firstBinding + i].size = 0;
      } else {
         struct tu_buffer *buf = tu_buffer_from_handle(pBuffers[i]);
         cmd->state.vb[firstBinding + i].base = buf->iova + pOffsets[i];
         cmd->state.vb[firstBinding + i].size =
            vk_buffer_range(&buf->vk, pOffsets[i], pSizes ? pSizes[i] : VK_WHOLE_SIZE);
      }
   }

   for (uint32_t i = 0; i < cmd->state.max_vbs_bound; i++) {
      tu_cs_emit_regs(&cs,
                      A6XX_VFD_FETCH_BASE(i, .qword = cmd->state.vb[i].base),
                      A6XX_VFD_FETCH_SIZE(i, cmd->state.vb[i].size));
   }

   cmd->state.dirty |= TU_CMD_DIRTY_VERTEX_BUFFERS;
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer,
                          VkBuffer buffer,
                          VkDeviceSize offset,
                          VkDeviceSize size,
                          VkIndexType indexType)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buf, buffer);

   size = vk_buffer_range(&buf->vk, offset, size);

   uint32_t index_size, index_shift, restart_index;

   switch (indexType) {
   case VK_INDEX_TYPE_UINT16:
      index_size = INDEX4_SIZE_16_BIT;
      index_shift = 1;
      restart_index = 0xffff;
      break;
   case VK_INDEX_TYPE_UINT32:
      index_size = INDEX4_SIZE_32_BIT;
      index_shift = 2;
      restart_index = 0xffffffff;
      break;
   case VK_INDEX_TYPE_UINT8_EXT:
      index_size = INDEX4_SIZE_8_BIT;
      index_shift = 0;
      restart_index = 0xff;
      break;
   default:
      unreachable("invalid VkIndexType");
   }

   /* initialize/update the restart index */
   if (cmd->state.index_size != index_size)
      tu_cs_emit_regs(&cmd->draw_cs, A6XX_PC_RESTART_INDEX(restart_index));

   cmd->state.index_va = buf->iova + offset;
   cmd->state.max_index_count = size >> index_shift;
   cmd->state.index_size = index_size;
}

template <chip CHIP>
static void
tu6_emit_descriptor_sets(struct tu_cmd_buffer *cmd,
                         VkPipelineBindPoint bind_point)
{
   struct tu_descriptor_state *descriptors_state =
      tu_get_descriptors_state(cmd, bind_point);
   uint32_t sp_bindless_base_reg, hlsq_bindless_base_reg;
   struct tu_cs *cs, state_cs;

   if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
      sp_bindless_base_reg = __SP_BINDLESS_BASE_DESCRIPTOR<CHIP>(0, {}).reg;
      hlsq_bindless_base_reg = REG_A6XX_HLSQ_BINDLESS_BASE(0);

      if (CHIP == A6XX) {
         cmd->state.desc_sets =
            tu_cs_draw_state(&cmd->sub_cs, &state_cs,
                             4 + 4 * descriptors_state->max_sets_bound +
                             (descriptors_state->max_dynamic_offset_size ? 6 : 0));
      } else {
         cmd->state.desc_sets =
            tu_cs_draw_state(&cmd->sub_cs, &state_cs,
                             3 + 2 * descriptors_state->max_sets_bound +
                             (descriptors_state->max_dynamic_offset_size ? 3 : 0));
      }
      cs = &state_cs;
   } else {
      assert(bind_point == VK_PIPELINE_BIND_POINT_COMPUTE);

      sp_bindless_base_reg = __SP_CS_BINDLESS_BASE_DESCRIPTOR<CHIP>(0, {}).reg;
      hlsq_bindless_base_reg = REG_A6XX_HLSQ_CS_BINDLESS_BASE(0);

      cs = &cmd->cs;
   }

   tu_cs_emit_pkt4(cs, sp_bindless_base_reg, 2 * descriptors_state->max_sets_bound);
   tu_cs_emit_array(cs, (const uint32_t*)descriptors_state->set_iova, 2 * descriptors_state->max_sets_bound);
   if (CHIP == A6XX) {
      tu_cs_emit_pkt4(cs, hlsq_bindless_base_reg, 2 * descriptors_state->max_sets_bound);
      tu_cs_emit_array(cs, (const uint32_t*)descriptors_state->set_iova, 2 * descriptors_state->max_sets_bound);
   }

   /* Dynamic descriptors get the reserved descriptor set. */
   if (descriptors_state->max_dynamic_offset_size) {
      int reserved_set_idx = cmd->device->physical_device->reserved_set_idx;
      assert(reserved_set_idx >= 0); /* reserved set must be bound */

      tu_cs_emit_pkt4(cs, sp_bindless_base_reg + reserved_set_idx * 2, 2);
      tu_cs_emit_qw(cs, descriptors_state->set_iova[reserved_set_idx]);
      if (CHIP == A6XX) {
         tu_cs_emit_pkt4(cs, hlsq_bindless_base_reg + reserved_set_idx * 2, 2);
         tu_cs_emit_qw(cs, descriptors_state->set_iova[reserved_set_idx]);
      }
   }

   tu_cs_emit_regs(cs, HLSQ_INVALIDATE_CMD(CHIP,
      .cs_bindless = bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ? CHIP == A6XX ? 0x1f : 0xff : 0,
      .gfx_bindless = bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS ? CHIP == A6XX ? 0x1f : 0xff : 0,
   ));

   if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
      assert(cs->cur == cs->end); /* validate draw state size */
      /* note: this also avoids emitting draw states before renderpass clears,
       * which may use the 3D clear path (for MSAA cases)
       */
      if (!(cmd->state.dirty & TU_CMD_DIRTY_DRAW_STATE)) {
         tu_cs_emit_pkt7(&cmd->draw_cs, CP_SET_DRAW_STATE, 3);
         tu_cs_emit_draw_state(&cmd->draw_cs, TU_DRAW_STATE_DESC_SETS, cmd->state.desc_sets);
      }
   }
}

/* We lazily emit the draw state for desciptor sets at draw time, so that we can
 * batch together multiple tu_CmdBindDescriptorSets() calls.  ANGLE and zink
 * will often emit multiple bind calls in a draw.
 */
static void
tu_dirty_desc_sets(struct tu_cmd_buffer *cmd,
                   VkPipelineBindPoint pipelineBindPoint)
{
   if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
      cmd->state.dirty |= TU_CMD_DIRTY_COMPUTE_DESC_SETS;
   } else {
      assert(pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS);
      cmd->state.dirty |= TU_CMD_DIRTY_DESC_SETS;
   }
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                         VkPipelineBindPoint pipelineBindPoint,
                         VkPipelineLayout _layout,
                         uint32_t firstSet,
                         uint32_t descriptorSetCount,
                         const VkDescriptorSet *pDescriptorSets,
                         uint32_t dynamicOffsetCount,
                         const uint32_t *pDynamicOffsets)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_pipeline_layout, layout, _layout);
   unsigned dyn_idx = 0;

   struct tu_descriptor_state *descriptors_state =
      tu_get_descriptors_state(cmd, pipelineBindPoint);

   descriptors_state->max_sets_bound =
      MAX2(descriptors_state->max_sets_bound, firstSet + descriptorSetCount);

   unsigned dynamic_offset_offset = 0;
   for (unsigned i = 0; i < firstSet; i++) {
      dynamic_offset_offset += layout->set[i].layout->dynamic_offset_size;
   }

   for (unsigned i = 0; i < descriptorSetCount; ++i) {
      unsigned idx = i + firstSet;
      TU_FROM_HANDLE(tu_descriptor_set, set, pDescriptorSets[i]);

      descriptors_state->sets[idx] = set;
      descriptors_state->set_iova[idx] = set ?
         (set->va | BINDLESS_DESCRIPTOR_64B) : 0;

      if (!set)
         continue;

      if (set->layout->has_inline_uniforms)
         cmd->state.dirty |= TU_CMD_DIRTY_SHADER_CONSTS;

      if (!set->layout->dynamic_offset_size)
         continue;

      uint32_t *src = set->dynamic_descriptors;
      uint32_t *dst = descriptors_state->dynamic_descriptors +
         dynamic_offset_offset / 4;
      for (unsigned j = 0; j < set->layout->binding_count; j++) {
         struct tu_descriptor_set_binding_layout *binding =
            &set->layout->binding[j];
         if (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
             binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            for (unsigned k = 0; k < binding->array_size; k++, dyn_idx++) {
               assert(dyn_idx < dynamicOffsetCount);
               uint32_t offset = pDynamicOffsets[dyn_idx];
               memcpy(dst, src, binding->size);

               if (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                  /* Note: we can assume here that the addition won't roll
                   * over and change the SIZE field.
                   */
                  uint64_t va = src[0] | ((uint64_t)src[1] << 32);
                  va += offset;
                  dst[0] = va;
                  dst[1] = va >> 32;
               } else {
                  uint32_t *dst_desc = dst;
                  for (unsigned i = 0;
                       i < binding->size / (4 * A6XX_TEX_CONST_DWORDS);
                       i++, dst_desc += A6XX_TEX_CONST_DWORDS) {
                     /* Note: A6XX_TEX_CONST_5_DEPTH is always 0 */
                     uint64_t va = dst_desc[4] | ((uint64_t)dst_desc[5] << 32);
                     uint32_t desc_offset =
                        (dst_desc[2] &
                         A6XX_TEX_CONST_2_STARTOFFSETTEXELS__MASK) >>
                        A6XX_TEX_CONST_2_STARTOFFSETTEXELS__SHIFT;

                     /* Without the ability to cast 16-bit as 32-bit, there is
                      * only one descriptor whose texels are 32 bits (4
                      * bytes). With casting, there are two descriptors, the
                      * first being 16-bit and the second being 32-bit.
                      */
                     unsigned offset_shift =
                        binding->size == 4 * A6XX_TEX_CONST_DWORDS || i == 1 ? 2 : 1;

                     va += desc_offset << offset_shift;
                     va += offset;
                     unsigned new_offset = (va & 0x3f) >> offset_shift;
                     va &= ~0x3full;
                     dst_desc[4] = va;
                     dst_desc[5] = va >> 32;
                     dst_desc[2] =
                        (dst_desc[2] & ~A6XX_TEX_CONST_2_STARTOFFSETTEXELS__MASK) |
                        A6XX_TEX_CONST_2_STARTOFFSETTEXELS(new_offset);
                  }
               }

               dst += binding->size / 4;
               src += binding->size / 4;
            }
         }
      }

      dynamic_offset_offset += layout->set[idx].layout->dynamic_offset_size;
   }
   assert(dyn_idx == dynamicOffsetCount);

   if (dynamic_offset_offset) {
      descriptors_state->max_dynamic_offset_size =
         MAX2(descriptors_state->max_dynamic_offset_size, dynamic_offset_offset);

      /* allocate and fill out dynamic descriptor set */
      struct tu_cs_memory dynamic_desc_set;
      int reserved_set_idx = cmd->device->physical_device->reserved_set_idx;
      VkResult result =
         tu_cs_alloc(&cmd->sub_cs,
                     descriptors_state->max_dynamic_offset_size /
                     (4 * A6XX_TEX_CONST_DWORDS),
                     A6XX_TEX_CONST_DWORDS, &dynamic_desc_set);
      if (result != VK_SUCCESS) {
         vk_command_buffer_set_error(&cmd->vk, result);
         return;
      }

      memcpy(dynamic_desc_set.map, descriptors_state->dynamic_descriptors,
             descriptors_state->max_dynamic_offset_size);
      assert(reserved_set_idx >= 0); /* reserved set must be bound */
      descriptors_state->set_iova[reserved_set_idx] = dynamic_desc_set.iova | BINDLESS_DESCRIPTOR_64B;
   }

   tu_dirty_desc_sets(cmd, pipelineBindPoint);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBindDescriptorBuffersEXT(
   VkCommandBuffer commandBuffer,
   uint32_t bufferCount,
   const VkDescriptorBufferBindingInfoEXT *pBindingInfos)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);

   for (unsigned i = 0; i < bufferCount; i++)
      cmd->state.descriptor_buffer_iova[i] = pBindingInfos[i].address;
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdSetDescriptorBufferOffsetsEXT(
   VkCommandBuffer commandBuffer,
   VkPipelineBindPoint pipelineBindPoint,
   VkPipelineLayout _layout,
   uint32_t firstSet,
   uint32_t setCount,
   const uint32_t *pBufferIndices,
   const VkDeviceSize *pOffsets)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_pipeline_layout, layout, _layout);

   struct tu_descriptor_state *descriptors_state =
      tu_get_descriptors_state(cmd, pipelineBindPoint);

   descriptors_state->max_sets_bound =
      MAX2(descriptors_state->max_sets_bound, firstSet + setCount);

   for (unsigned i = 0; i < setCount; ++i) {
      unsigned idx = i + firstSet;
      struct tu_descriptor_set_layout *set_layout = layout->set[idx].layout;

      descriptors_state->set_iova[idx] =
         (cmd->state.descriptor_buffer_iova[pBufferIndices[i]] + pOffsets[i]) |
         BINDLESS_DESCRIPTOR_64B;

      if (set_layout->has_inline_uniforms)
         cmd->state.dirty |= TU_CMD_DIRTY_SHADER_CONSTS;
   }

   tu_dirty_desc_sets(cmd, pipelineBindPoint);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBindDescriptorBufferEmbeddedSamplersEXT(
   VkCommandBuffer commandBuffer,
   VkPipelineBindPoint pipelineBindPoint,
   VkPipelineLayout _layout,
   uint32_t set)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_pipeline_layout, layout, _layout);

   struct tu_descriptor_set_layout *set_layout = layout->set[set].layout;

   struct tu_descriptor_state *descriptors_state =
      tu_get_descriptors_state(cmd, pipelineBindPoint);

   descriptors_state->max_sets_bound =
      MAX2(descriptors_state->max_sets_bound, set + 1);

   descriptors_state->set_iova[set] = set_layout->embedded_samplers->iova |
         BINDLESS_DESCRIPTOR_64B;

   tu_dirty_desc_sets(cmd, pipelineBindPoint);
}

static enum VkResult
tu_push_descriptor_set_update_layout(struct tu_device *device,
                                     struct tu_descriptor_set *set,
                                     struct tu_descriptor_set_layout *layout)
{
   if (set->layout == layout)
      return VK_SUCCESS;

   if (set->layout)
      vk_descriptor_set_layout_unref(&device->vk, &set->layout->vk);
   vk_descriptor_set_layout_ref(&layout->vk);
   set->layout = layout;

   if (set->host_size < layout->size) {
      void *new_buf =
         vk_realloc(&device->vk.alloc, set->mapped_ptr, layout->size, 8,
                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!new_buf)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      set->mapped_ptr = (uint32_t *) new_buf;
      set->host_size = layout->size;
   }
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                           VkPipelineBindPoint pipelineBindPoint,
                           VkPipelineLayout _layout,
                           uint32_t _set,
                           uint32_t descriptorWriteCount,
                           const VkWriteDescriptorSet *pDescriptorWrites)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_pipeline_layout, pipe_layout, _layout);
   struct tu_descriptor_set_layout *layout = pipe_layout->set[_set].layout;
   struct tu_descriptor_set *set =
      &tu_get_descriptors_state(cmd, pipelineBindPoint)->push_set;

   struct tu_cs_memory set_mem;
   VkResult result = tu_cs_alloc(&cmd->sub_cs,
                                 DIV_ROUND_UP(layout->size, A6XX_TEX_CONST_DWORDS * 4),
                                 A6XX_TEX_CONST_DWORDS, &set_mem);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   result = tu_push_descriptor_set_update_layout(cmd->device, set, layout);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   tu_update_descriptor_sets(cmd->device, tu_descriptor_set_to_handle(set),
                             descriptorWriteCount, pDescriptorWrites, 0, NULL);

   memcpy(set_mem.map, set->mapped_ptr, layout->size);
   set->va = set_mem.iova;

   const VkDescriptorSet desc_set[] = { tu_descriptor_set_to_handle(set) };
   tu_CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, _layout, _set,
                            1, desc_set, 0, NULL);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                       VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                       VkPipelineLayout _layout,
                                       uint32_t _set,
                                       const void* pData)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_pipeline_layout, pipe_layout, _layout);
   TU_FROM_HANDLE(tu_descriptor_update_template, templ, descriptorUpdateTemplate);
   struct tu_descriptor_set_layout *layout = pipe_layout->set[_set].layout;
   struct tu_descriptor_set *set =
      &tu_get_descriptors_state(cmd, templ->bind_point)->push_set;

   struct tu_cs_memory set_mem;
   VkResult result = tu_cs_alloc(&cmd->sub_cs,
                                 DIV_ROUND_UP(layout->size, A6XX_TEX_CONST_DWORDS * 4),
                                 A6XX_TEX_CONST_DWORDS, &set_mem);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   result = tu_push_descriptor_set_update_layout(cmd->device, set, layout);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   tu_update_descriptor_set_with_template(cmd->device, set, descriptorUpdateTemplate, pData);

   memcpy(set_mem.map, set->mapped_ptr, layout->size);
   set->va = set_mem.iova;

   const VkDescriptorSet desc_set[] = { tu_descriptor_set_to_handle(set) };
   tu_CmdBindDescriptorSets(commandBuffer, templ->bind_point, _layout, _set,
                            1, desc_set, 0, NULL);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer,
                                      uint32_t firstBinding,
                                      uint32_t bindingCount,
                                      const VkBuffer *pBuffers,
                                      const VkDeviceSize *pOffsets,
                                      const VkDeviceSize *pSizes)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   /* using COND_REG_EXEC for xfb commands matches the blob behavior
    * presumably there isn't any benefit using a draw state when the
    * condition is (SYSMEM | BINNING)
    */
   tu_cond_exec_start(cs, CP_COND_REG_EXEC_0_MODE(RENDER_MODE) |
                          CP_COND_REG_EXEC_0_SYSMEM |
                          CP_COND_REG_EXEC_0_BINNING);

   for (uint32_t i = 0; i < bindingCount; i++) {
      TU_FROM_HANDLE(tu_buffer, buf, pBuffers[i]);
      uint64_t iova = buf->iova + pOffsets[i];
      uint32_t size = buf->bo->size - (iova - buf->bo->iova);
      uint32_t idx = i + firstBinding;

      if (pSizes && pSizes[i] != VK_WHOLE_SIZE)
         size = pSizes[i];

      /* BUFFER_BASE is 32-byte aligned, add remaining offset to BUFFER_OFFSET */
      uint32_t offset = iova & 0x1f;
      iova &= ~(uint64_t) 0x1f;

      tu_cs_emit_pkt4(cs, REG_A6XX_VPC_SO_BUFFER_BASE(idx), 3);
      tu_cs_emit_qw(cs, iova);
      tu_cs_emit(cs, size + offset);

      cmd->state.streamout_offset[idx] = offset;
   }

   tu_cond_exec_end(cs);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                                uint32_t firstCounterBuffer,
                                uint32_t counterBufferCount,
                                const VkBuffer *pCounterBuffers,
                                const VkDeviceSize *pCounterBufferOffsets)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu_cond_exec_start(cs, CP_COND_REG_EXEC_0_MODE(RENDER_MODE) |
                          CP_COND_REG_EXEC_0_SYSMEM |
                          CP_COND_REG_EXEC_0_BINNING);

   tu_cs_emit_regs(cs, A6XX_VPC_SO_DISABLE(false));

   /* TODO: only update offset for active buffers */
   for (uint32_t i = 0; i < IR3_MAX_SO_BUFFERS; i++)
      tu_cs_emit_regs(cs, A6XX_VPC_SO_BUFFER_OFFSET(i, cmd->state.streamout_offset[i]));

   for (uint32_t i = 0; i < (pCounterBuffers ? counterBufferCount : 0); i++) {
      uint32_t idx = firstCounterBuffer + i;
      uint32_t offset = cmd->state.streamout_offset[idx];
      uint64_t counter_buffer_offset = pCounterBufferOffsets ? pCounterBufferOffsets[i] : 0u;

      if (!pCounterBuffers[i])
         continue;

      TU_FROM_HANDLE(tu_buffer, buf, pCounterBuffers[i]);

      tu_cs_emit_pkt7(cs, CP_MEM_TO_REG, 3);
      tu_cs_emit(cs, CP_MEM_TO_REG_0_REG(REG_A6XX_VPC_SO_BUFFER_OFFSET(idx)) |
                     CP_MEM_TO_REG_0_UNK31 |
                     CP_MEM_TO_REG_0_CNT(1));
      tu_cs_emit_qw(cs, buf->iova + counter_buffer_offset);

      if (offset) {
         tu_cs_emit_pkt7(cs, CP_REG_RMW, 3);
         tu_cs_emit(cs, CP_REG_RMW_0_DST_REG(REG_A6XX_VPC_SO_BUFFER_OFFSET(idx)) |
                        CP_REG_RMW_0_SRC1_ADD);
         tu_cs_emit(cs, 0xffffffff);
         tu_cs_emit(cs, offset);
      }
   }

   tu_cond_exec_end(cs);
}

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                              uint32_t firstCounterBuffer,
                              uint32_t counterBufferCount,
                              const VkBuffer *pCounterBuffers,
                              const VkDeviceSize *pCounterBufferOffsets)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu_cond_exec_start(cs, CP_COND_REG_EXEC_0_MODE(RENDER_MODE) |
                          CP_COND_REG_EXEC_0_SYSMEM |
                          CP_COND_REG_EXEC_0_BINNING);

   tu_cs_emit_regs(cs, A6XX_VPC_SO_DISABLE(true));

   /* TODO: only flush buffers that need to be flushed */
   for (uint32_t i = 0; i < IR3_MAX_SO_BUFFERS; i++) {
      /* note: FLUSH_BASE is always the same, so it could go in init_hw()? */
      tu_cs_emit_pkt4(cs, REG_A6XX_VPC_SO_FLUSH_BASE(i), 2);
      tu_cs_emit_qw(cs, global_iova_arr(cmd, flush_base, i));
      tu_emit_event_write<CHIP>(cmd, cs, (enum fd_gpu_event) (FD_FLUSH_SO_0 + i));
   }

   for (uint32_t i = 0; i < (pCounterBuffers ? counterBufferCount : 0); i++) {
      uint32_t idx = firstCounterBuffer + i;
      uint32_t offset = cmd->state.streamout_offset[idx];
      uint64_t counter_buffer_offset = pCounterBufferOffsets ? pCounterBufferOffsets[i] : 0u;

      if (!pCounterBuffers[i])
         continue;

      TU_FROM_HANDLE(tu_buffer, buf, pCounterBuffers[i]);

      /* VPC_SO_FLUSH_BASE has dwords counter, but counter should be in bytes */
      tu_cs_emit_pkt7(cs, CP_MEM_TO_REG, 3);
      tu_cs_emit(cs, CP_MEM_TO_REG_0_REG(REG_A6XX_CP_SCRATCH_REG(0)) |
                     COND(CHIP == A6XX, CP_MEM_TO_REG_0_SHIFT_BY_2) |
                     0x40000 | /* ??? */
                     CP_MEM_TO_REG_0_UNK31 |
                     CP_MEM_TO_REG_0_CNT(1));
      tu_cs_emit_qw(cs, global_iova_arr(cmd, flush_base, idx));

      if (offset) {
         tu_cs_emit_pkt7(cs, CP_REG_RMW, 3);
         tu_cs_emit(cs, CP_REG_RMW_0_DST_REG(REG_A6XX_CP_SCRATCH_REG(0)) |
                        CP_REG_RMW_0_SRC1_ADD);
         tu_cs_emit(cs, 0xffffffff);
         tu_cs_emit(cs, -offset);
      }

      tu_cs_emit_pkt7(cs, CP_REG_TO_MEM, 3);
      tu_cs_emit(cs, CP_REG_TO_MEM_0_REG(REG_A6XX_CP_SCRATCH_REG(0)) |
                     CP_REG_TO_MEM_0_CNT(1));
      tu_cs_emit_qw(cs, buf->iova + counter_buffer_offset);
   }

   tu_cond_exec_end(cs);

   cmd->state.rp.xfb_used = true;
}
TU_GENX(tu_CmdEndTransformFeedbackEXT);

VKAPI_ATTR void VKAPI_CALL
tu_CmdPushConstants(VkCommandBuffer commandBuffer,
                    VkPipelineLayout layout,
                    VkShaderStageFlags stageFlags,
                    uint32_t offset,
                    uint32_t size,
                    const void *pValues)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   memcpy((char *) cmd->push_constants + offset, pValues, size);
   cmd->state.dirty |= TU_CMD_DIRTY_SHADER_CONSTS;
}

/* Flush everything which has been made available but we haven't actually
 * flushed yet.
 */
static void
tu_flush_all_pending(struct tu_cache_state *cache)
{
   cache->flush_bits |= cache->pending_flush_bits & TU_CMD_FLAG_ALL_FLUSH;
   cache->pending_flush_bits &= ~TU_CMD_FLAG_ALL_FLUSH;
}

template <chip CHIP>
VKAPI_ATTR VkResult VKAPI_CALL
tu_EndCommandBuffer(VkCommandBuffer commandBuffer)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, commandBuffer);

   /* We currently flush CCU at the end of the command buffer, like
    * what the blob does. There's implicit synchronization around every
    * vkQueueSubmit, but the kernel only flushes the UCHE, and we don't
    * know yet if this command buffer will be the last in the submit so we
    * have to defensively flush everything else.
    *
    * TODO: We could definitely do better than this, since these flushes
    * aren't required by Vulkan, but we'd need kernel support to do that.
    * Ideally, we'd like the kernel to flush everything afterwards, so that we
    * wouldn't have to do any flushes here, and when submitting multiple
    * command buffers there wouldn't be any unnecessary flushes in between.
    */
   if (cmd_buffer->state.pass) {
      tu_flush_all_pending(&cmd_buffer->state.renderpass_cache);
      tu_emit_cache_flush_renderpass<CHIP>(cmd_buffer);

      trace_end_cmd_buffer(&cmd_buffer->trace, &cmd_buffer->draw_cs);
   } else {
      tu_flush_all_pending(&cmd_buffer->state.cache);
      cmd_buffer->state.cache.flush_bits |=
         TU_CMD_FLAG_CCU_FLUSH_COLOR |
         TU_CMD_FLAG_CCU_FLUSH_DEPTH;
      tu_emit_cache_flush<CHIP>(cmd_buffer);

      trace_end_cmd_buffer(&cmd_buffer->trace, &cmd_buffer->cs);
   }

   tu_cs_end(&cmd_buffer->cs);
   tu_cs_end(&cmd_buffer->draw_cs);
   tu_cs_end(&cmd_buffer->draw_epilogue_cs);

   return vk_command_buffer_end(&cmd_buffer->vk);
}
TU_GENX(tu_EndCommandBuffer);

static void
tu_bind_vs(struct tu_cmd_buffer *cmd, struct tu_shader *vs)
{
   cmd->state.shaders[MESA_SHADER_VERTEX] = vs;
}

static void
tu_bind_tcs(struct tu_cmd_buffer *cmd, struct tu_shader *tcs)
{
   cmd->state.shaders[MESA_SHADER_TESS_CTRL] = tcs;
}

static void
tu_bind_tes(struct tu_cmd_buffer *cmd, struct tu_shader *tes)
{
   if (cmd->state.shaders[MESA_SHADER_TESS_EVAL] != tes) {
      cmd->state.shaders[MESA_SHADER_TESS_EVAL] = tes;
      cmd->state.dirty |= TU_CMD_DIRTY_TES;

      if (!cmd->state.tess_params.valid ||
          cmd->state.tess_params.output_upper_left !=
          tes->tes.tess_output_upper_left ||
          cmd->state.tess_params.output_lower_left !=
          tes->tes.tess_output_lower_left ||
          cmd->state.tess_params.spacing != tes->tes.tess_spacing) {
         cmd->state.tess_params.output_upper_left =
            tes->tes.tess_output_upper_left;
         cmd->state.tess_params.output_lower_left =
            tes->tes.tess_output_lower_left;
         cmd->state.tess_params.spacing = tes->tes.tess_spacing;
         cmd->state.tess_params.valid = true;
         cmd->state.dirty |= TU_CMD_DIRTY_TESS_PARAMS;
      }
   }
}

static void
tu_bind_gs(struct tu_cmd_buffer *cmd, struct tu_shader *gs)
{
   cmd->state.shaders[MESA_SHADER_GEOMETRY] = gs;
}

static void
tu_bind_fs(struct tu_cmd_buffer *cmd, struct tu_shader *fs)
{
   if (cmd->state.shaders[MESA_SHADER_FRAGMENT] != fs) {
      cmd->state.shaders[MESA_SHADER_FRAGMENT] = fs;
      cmd->state.dirty |= TU_CMD_DIRTY_LRZ;
   }
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdBindPipeline(VkCommandBuffer commandBuffer,
                   VkPipelineBindPoint pipelineBindPoint,
                   VkPipeline _pipeline)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_pipeline, pipeline, _pipeline);

   if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
      cmd->state.shaders[MESA_SHADER_COMPUTE] =
         pipeline->shaders[MESA_SHADER_COMPUTE];
      tu_cs_emit_state_ib(&cmd->cs,
                          pipeline->shaders[MESA_SHADER_COMPUTE]->state);
      cmd->state.compute_load_state = pipeline->load_state;
      return;
   }

   assert(pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS);

   struct tu_graphics_pipeline *gfx_pipeline = tu_pipeline_to_graphics(pipeline);
   cmd->state.dirty |= TU_CMD_DIRTY_DESC_SETS | TU_CMD_DIRTY_SHADER_CONSTS |
                       TU_CMD_DIRTY_VS_PARAMS | TU_CMD_DIRTY_PROGRAM;

   tu_bind_vs(cmd, pipeline->shaders[MESA_SHADER_VERTEX]);
   tu_bind_tcs(cmd, pipeline->shaders[MESA_SHADER_TESS_CTRL]);
   tu_bind_tes(cmd, pipeline->shaders[MESA_SHADER_TESS_EVAL]);
   tu_bind_gs(cmd, pipeline->shaders[MESA_SHADER_GEOMETRY]);
   tu_bind_fs(cmd, pipeline->shaders[MESA_SHADER_FRAGMENT]);

   /* We precompile static state and count it as dynamic, so we have to
    * manually clear bitset that tells which dynamic state is set, in order to
    * make sure that future dynamic state will be emitted. The issue is that
    * framework remembers only a past REAL dynamic state and compares a new
    * dynamic state against it, and not against our static state masquaraded
    * as dynamic.
    */
   BITSET_ANDNOT(cmd->vk.dynamic_graphics_state.set,
                 cmd->vk.dynamic_graphics_state.set,
                 pipeline->static_state_mask);

   vk_cmd_set_dynamic_graphics_state(&cmd->vk,
                                     &gfx_pipeline->dynamic_state);
   cmd->state.program = pipeline->program;

   cmd->state.load_state = pipeline->load_state;
   cmd->state.prim_order_sysmem = pipeline->prim_order.state_sysmem;
   cmd->state.prim_order_gmem = pipeline->prim_order.state_gmem;

   if (gfx_pipeline->feedback_loop_may_involve_textures &&
       !cmd->state.rp.disable_gmem) {
      /* VK_EXT_attachment_feedback_loop_layout allows feedback loop to involve
       * not only input attachments but also sampled images or image resources.
       * But we cannot just patch gmem for image in the descriptors.
       *
       * At the moment, in context of DXVK, it is expected that only a few
       * drawcalls in a frame would use feedback loop and they would be wrapped
       * in their own renderpasses, so it should be ok to force sysmem.
       *
       * However, there are two further possible optimizations if need would
       * arise for other translation layer:
       * - Tiling could be enabled if we ensure that there is no barrier in
       *   the renderpass;
       * - Check that both pipeline and attachments agree that feedback loop
       *   is needed.
       */
      perf_debug(
         cmd->device,
         "Disabling gmem due to VK_EXT_attachment_feedback_loop_layout");
      cmd->state.rp.disable_gmem = true;
   }

   if (pipeline->prim_order.sysmem_single_prim_mode &&
       !cmd->state.rp.sysmem_single_prim_mode) {
      if (gfx_pipeline->feedback_loop_color ||
          gfx_pipeline->feedback_loop_ds) {
         perf_debug(cmd->device, "single_prim_mode due to feedback loop");
      } else {
         perf_debug(cmd->device, "single_prim_mode due to rast order access");
      }
      cmd->state.rp.sysmem_single_prim_mode = true;
   }

   if (pipeline->lrz_blend.valid) {
      if (cmd->state.blend_reads_dest != pipeline->lrz_blend.reads_dest) {
         cmd->state.blend_reads_dest = pipeline->lrz_blend.reads_dest;
         cmd->state.dirty |= TU_CMD_DIRTY_LRZ;
      }
   }
   cmd->state.pipeline_blend_lrz = pipeline->lrz_blend.valid;

   if (pipeline->bandwidth.valid)
      cmd->state.bandwidth = pipeline->bandwidth;
   cmd->state.pipeline_bandwidth = pipeline->bandwidth.valid;

   struct tu_cs *cs = &cmd->draw_cs;

   /* note: this also avoids emitting draw states before renderpass clears,
    * which may use the 3D clear path (for MSAA cases)
    */
   if (!(cmd->state.dirty & TU_CMD_DIRTY_DRAW_STATE)) {
      uint32_t mask = pipeline->set_state_mask;

      tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 3 * (11 + util_bitcount(mask)));
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_PROGRAM_CONFIG, pipeline->program.config_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS, pipeline->program.vs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS_BINNING, pipeline->program.vs_binning_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_HS, pipeline->program.hs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DS, pipeline->program.ds_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_GS, pipeline->program.gs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_GS_BINNING, pipeline->program.gs_binning_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_FS, pipeline->program.fs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VPC, pipeline->program.vpc_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_PRIM_MODE_SYSMEM, pipeline->prim_order.state_sysmem);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_PRIM_MODE_GMEM, pipeline->prim_order.state_gmem);

      u_foreach_bit(i, mask)
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DYNAMIC + i, pipeline->dynamic_state[i]);
   }

   cmd->state.pipeline_draw_states = pipeline->set_state_mask;
   u_foreach_bit(i, pipeline->set_state_mask)
      cmd->state.dynamic_state[i] = pipeline->dynamic_state[i];

   if (pipeline->active_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
      cmd->state.rp.has_tess = true;
   }

   if (pipeline->program.per_view_viewport != cmd->state.per_view_viewport) {
      cmd->state.per_view_viewport = pipeline->program.per_view_viewport;
      cmd->state.dirty |= TU_CMD_DIRTY_PER_VIEW_VIEWPORT;
   }

   if (gfx_pipeline->feedback_loop_ds != cmd->state.pipeline_feedback_loop_ds) {
      cmd->state.pipeline_feedback_loop_ds = gfx_pipeline->feedback_loop_ds;
      cmd->state.dirty |= TU_CMD_DIRTY_LRZ;
   }
}

static void
tu_flush_for_access(struct tu_cache_state *cache,
                    enum tu_cmd_access_mask src_mask,
                    enum tu_cmd_access_mask dst_mask)
{
   BITMASK_ENUM(tu_cmd_flush_bits) flush_bits = 0;

   if (src_mask & TU_ACCESS_SYSMEM_WRITE) {
      cache->pending_flush_bits |= TU_CMD_FLAG_ALL_INVALIDATE;
   }

   if (src_mask & TU_ACCESS_CP_WRITE) {
      /* Flush the CP write queue.
       */
      cache->pending_flush_bits |=
         TU_CMD_FLAG_WAIT_MEM_WRITES |
         TU_CMD_FLAG_ALL_INVALIDATE;
   }

#define SRC_FLUSH(domain, flush, invalidate) \
   if (src_mask & TU_ACCESS_##domain##_WRITE) {                      \
      cache->pending_flush_bits |= TU_CMD_FLAG_##flush |             \
         (TU_CMD_FLAG_ALL_INVALIDATE & ~TU_CMD_FLAG_##invalidate);   \
   }

   SRC_FLUSH(UCHE, CACHE_FLUSH, CACHE_INVALIDATE)
   SRC_FLUSH(CCU_COLOR, CCU_FLUSH_COLOR, CCU_INVALIDATE_COLOR)
   SRC_FLUSH(CCU_DEPTH, CCU_FLUSH_DEPTH, CCU_INVALIDATE_DEPTH)

#undef SRC_FLUSH

#define SRC_INCOHERENT_FLUSH(domain, flush, invalidate)              \
   if (src_mask & TU_ACCESS_##domain##_INCOHERENT_WRITE) {           \
      flush_bits |= TU_CMD_FLAG_##flush;                             \
      cache->pending_flush_bits |=                                   \
         (TU_CMD_FLAG_ALL_INVALIDATE & ~TU_CMD_FLAG_##invalidate);   \
   }

   SRC_INCOHERENT_FLUSH(CCU_COLOR, CCU_FLUSH_COLOR, CCU_INVALIDATE_COLOR)
   SRC_INCOHERENT_FLUSH(CCU_DEPTH, CCU_FLUSH_DEPTH, CCU_INVALIDATE_DEPTH)

#undef SRC_INCOHERENT_FLUSH

   /* Treat host & sysmem write accesses the same, since the kernel implicitly
    * drains the queue before signalling completion to the host.
    */
   if (dst_mask & (TU_ACCESS_SYSMEM_READ | TU_ACCESS_SYSMEM_WRITE)) {
      flush_bits |= cache->pending_flush_bits & TU_CMD_FLAG_ALL_FLUSH;
   }

#define DST_FLUSH(domain, flush, invalidate) \
   if (dst_mask & (TU_ACCESS_##domain##_READ |                 \
                   TU_ACCESS_##domain##_WRITE)) {              \
      flush_bits |= cache->pending_flush_bits &                \
         (TU_CMD_FLAG_##invalidate |                           \
          (TU_CMD_FLAG_ALL_FLUSH & ~TU_CMD_FLAG_##flush));     \
   }

   DST_FLUSH(UCHE, CACHE_FLUSH, CACHE_INVALIDATE)
   DST_FLUSH(CCU_COLOR, CCU_FLUSH_COLOR, CCU_INVALIDATE_COLOR)
   DST_FLUSH(CCU_DEPTH, CCU_FLUSH_DEPTH, CCU_INVALIDATE_DEPTH)

#undef DST_FLUSH

#define DST_INCOHERENT_FLUSH(domain, flush, invalidate) \
   if (dst_mask & (TU_ACCESS_##domain##_INCOHERENT_READ |      \
                   TU_ACCESS_##domain##_INCOHERENT_WRITE)) {   \
      flush_bits |= TU_CMD_FLAG_##invalidate |                 \
          (cache->pending_flush_bits &                         \
           (TU_CMD_FLAG_ALL_FLUSH & ~TU_CMD_FLAG_##flush));    \
   }

   DST_INCOHERENT_FLUSH(CCU_COLOR, CCU_FLUSH_COLOR, CCU_INVALIDATE_COLOR)
   DST_INCOHERENT_FLUSH(CCU_DEPTH, CCU_FLUSH_DEPTH, CCU_INVALIDATE_DEPTH)

   if (dst_mask & TU_ACCESS_BINDLESS_DESCRIPTOR_READ) {
      flush_bits |= TU_CMD_FLAG_BINDLESS_DESCRIPTOR_INVALIDATE;
   }

#undef DST_INCOHERENT_FLUSH

   cache->flush_bits |= flush_bits;
   cache->pending_flush_bits &= ~flush_bits;
}

/* When translating Vulkan access flags to which cache is accessed
 * (CCU/UCHE/sysmem), we should take into account both the access flags and
 * the stage so that accesses with MEMORY_READ_BIT/MEMORY_WRITE_BIT + a
 * specific stage return something sensible. The specification for
 * VK_KHR_synchronization2 says that we should do this:
 *
 *    Additionally, scoping the pipeline stages into the barrier structs
 *    allows the use of the MEMORY_READ and MEMORY_WRITE flags without
 *    sacrificing precision. The per-stage access flags should be used to
 *    disambiguate specific accesses in a given stage or set of stages - for
 *    instance, between uniform reads and sampling operations.
 *
 * Note that while in all known cases the stage is actually enough, we should
 * still narrow things down based on the access flags to handle "old-style"
 * barriers that may specify a wider range of stages but more precise access
 * flags. These helpers allow us to do both.
 */

static bool
filter_read_access(VkAccessFlags2 flags, VkPipelineStageFlags2 stages,
                   VkAccessFlags2 tu_flags, VkPipelineStageFlags2 tu_stages)
{
   return (flags & (tu_flags | VK_ACCESS_2_MEMORY_READ_BIT)) &&
      (stages & (tu_stages | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
}

static bool
filter_write_access(VkAccessFlags2 flags, VkPipelineStageFlags2 stages,
                    VkAccessFlags2 tu_flags, VkPipelineStageFlags2 tu_stages)
{
   return (flags & (tu_flags | VK_ACCESS_2_MEMORY_WRITE_BIT)) &&
      (stages & (tu_stages | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
}

static bool
gfx_read_access(VkAccessFlags2 flags, VkPipelineStageFlags2 stages,
                VkAccessFlags2 tu_flags, VkPipelineStageFlags2 tu_stages)
{
   return filter_read_access(flags, stages, tu_flags,
                             tu_stages | VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
}

static bool
gfx_write_access(VkAccessFlags2 flags, VkPipelineStageFlags2 stages,
                 VkAccessFlags2 tu_flags, VkPipelineStageFlags2 tu_stages)
{
   return filter_write_access(flags, stages, tu_flags,
                              tu_stages | VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
}
static enum tu_cmd_access_mask
vk2tu_access(VkAccessFlags2 flags, VkPipelineStageFlags2 stages, bool image_only, bool gmem)
{
   BITMASK_ENUM(tu_cmd_access_mask) mask = 0;

   if (gfx_read_access(flags, stages,
                       VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT |
                       VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT |
                       VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT |
                       VK_ACCESS_2_HOST_READ_BIT,
                       VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT |
                       VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT |
                       VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT |
                       VK_PIPELINE_STAGE_2_HOST_BIT))
      mask |= TU_ACCESS_SYSMEM_READ;

   if (gfx_write_access(flags, stages,
                        VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
                        VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT))
      mask |= TU_ACCESS_CP_WRITE;

   if (gfx_write_access(flags, stages,
                        VK_ACCESS_2_HOST_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_HOST_BIT))
      mask |= TU_ACCESS_SYSMEM_WRITE;

#define SHADER_STAGES \
   (VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | \
    VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT | \
    VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT | \
    VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT | \
    VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT | \
    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | \
    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT)


   if (gfx_read_access(flags, stages,
                       VK_ACCESS_2_INDEX_READ_BIT |
                       VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT |
                       VK_ACCESS_2_UNIFORM_READ_BIT |
                       VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT |
                       VK_ACCESS_2_SHADER_READ_BIT,
                       VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT |
                       VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT |
                       VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT |
                       SHADER_STAGES))
       mask |= TU_ACCESS_UCHE_READ;

   if (gfx_read_access(flags, stages,
                       VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT,
                       SHADER_STAGES)) {
      mask |= TU_ACCESS_UCHE_READ | TU_ACCESS_BINDLESS_DESCRIPTOR_READ;
   }

   if (gfx_write_access(flags, stages,
                        VK_ACCESS_2_SHADER_WRITE_BIT |
                        VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
                        VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT |
                        SHADER_STAGES))
       mask |= TU_ACCESS_UCHE_WRITE;

   /* When using GMEM, the CCU is always flushed automatically to GMEM, and
    * then GMEM is flushed to sysmem. Furthermore, we already had to flush any
    * previous writes in sysmem mode when transitioning to GMEM. Therefore we
    * can ignore CCU and pretend that color attachments and transfers use
    * sysmem directly.
    */

   if (gfx_read_access(flags, stages,
                       VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                       VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT)) {
      if (gmem)
         mask |= TU_ACCESS_SYSMEM_READ;
      else
         mask |= TU_ACCESS_CCU_COLOR_INCOHERENT_READ;
   }

   if (gfx_read_access(flags, stages,
                       VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                       VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                       VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT)) {
      if (gmem)
         mask |= TU_ACCESS_SYSMEM_READ;
      else
         mask |= TU_ACCESS_CCU_DEPTH_INCOHERENT_READ;
   }

   if (gfx_write_access(flags, stages,
                        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT)) {
      if (gmem) {
         mask |= TU_ACCESS_SYSMEM_WRITE;
      } else {
         mask |= TU_ACCESS_CCU_COLOR_INCOHERENT_WRITE;
      }
   }

   if (gfx_write_access(flags, stages,
                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                        VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT)) {
      if (gmem) {
         mask |= TU_ACCESS_SYSMEM_WRITE;
      } else {
         mask |= TU_ACCESS_CCU_DEPTH_INCOHERENT_WRITE;
      }
   }

   if (filter_write_access(flags, stages,
                           VK_ACCESS_2_TRANSFER_WRITE_BIT,
                           VK_PIPELINE_STAGE_2_COPY_BIT |
                           VK_PIPELINE_STAGE_2_BLIT_BIT |
                           VK_PIPELINE_STAGE_2_CLEAR_BIT |
                           VK_PIPELINE_STAGE_2_RESOLVE_BIT |
                           VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT)) {
      if (gmem) {
         mask |= TU_ACCESS_SYSMEM_WRITE;
      } else if (image_only) {
         /* Because we always split up blits/copies of images involving
          * multiple layers, we always access each layer in the same way, with
          * the same base address, same format, etc. This means we can avoid
          * flushing between multiple writes to the same image. This elides
          * flushes between e.g. multiple blits to the same image.
          */
         mask |= TU_ACCESS_CCU_COLOR_WRITE;
      } else {
         mask |= TU_ACCESS_CCU_COLOR_INCOHERENT_WRITE;
      }
   }

   if (filter_read_access(flags, stages,
                          VK_ACCESS_2_TRANSFER_READ_BIT,
                          VK_PIPELINE_STAGE_2_COPY_BIT |
                          VK_PIPELINE_STAGE_2_BLIT_BIT |
                          VK_PIPELINE_STAGE_2_RESOLVE_BIT |
                          VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT)) {
      mask |= TU_ACCESS_UCHE_READ;
   }

   return mask;
}

/* These helpers deal with legacy BOTTOM_OF_PIPE/TOP_OF_PIPE stages.
 */

static VkPipelineStageFlags2
sanitize_src_stage(VkPipelineStageFlags2 stage_mask)
{
   /* From the Vulkan spec:
    *
    *    VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT is ...  equivalent to
    *    VK_PIPELINE_STAGE_2_NONE in the first scope.
    *
    *    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT is equivalent to
    *    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT with VkAccessFlags2 set to 0
    *    when specified in the first synchronization scope, ...
    */
   if (stage_mask & VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT)
      return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

   return stage_mask & ~VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
}

static VkPipelineStageFlags2
sanitize_dst_stage(VkPipelineStageFlags2 stage_mask)
{
   /* From the Vulkan spec:
    *
    *    VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT is equivalent to
    *    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT with VkAccessFlags2 set to 0
    *    when specified in the second synchronization scope, ...
    *
    *    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT is ... equivalent to
    *    VK_PIPELINE_STAGE_2_NONE in the second scope.
    *
    */
   if (stage_mask & VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT)
      return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

   return stage_mask & ~VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
}

static enum tu_stage
vk2tu_single_stage(VkPipelineStageFlags2 vk_stage, bool dst)
{
   /* If the destination stage is executed on the CP, then the CP also has to
    * wait for any WFI's to finish. This is already done for draw calls,
    * including before indirect param reads, for the most part, so we just
    * need to WFI and can use TU_STAGE_GPU.
    *
    * However, some indirect draw opcodes, depending on firmware, don't have
    * implicit CP_WAIT_FOR_ME so we have to handle it manually.
    *
    * Transform feedback counters are read via CP_MEM_TO_REG, which implicitly
    * does CP_WAIT_FOR_ME, so we don't include them here.
    *
    * Currently we read the draw predicate using CP_MEM_TO_MEM, which
    * also implicitly does CP_WAIT_FOR_ME. However CP_DRAW_PRED_SET does *not*
    * implicitly do CP_WAIT_FOR_ME, it seems to only wait for counters to
    * complete since it's written for DX11 where you can only predicate on the
    * result of a query object. So if we implement 64-bit comparisons in the
    * future, or if CP_DRAW_PRED_SET grows the capability to do 32-bit
    * comparisons, then this will have to be dealt with.
    */
   if (vk_stage == VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT ||
       vk_stage == VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT ||
       vk_stage == VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT)
      return TU_STAGE_CP;

   if (vk_stage == VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT ||
       vk_stage == VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)
      return dst ? TU_STAGE_CP : TU_STAGE_GPU;

   if (vk_stage == VK_PIPELINE_STAGE_2_HOST_BIT)
      return dst ? TU_STAGE_BOTTOM : TU_STAGE_CP;

   return TU_STAGE_GPU;
}

static enum tu_stage
vk2tu_src_stage(VkPipelineStageFlags2 vk_stages)
{
   enum tu_stage stage = TU_STAGE_CP;
   u_foreach_bit64 (bit, vk_stages) {
      enum tu_stage new_stage = vk2tu_single_stage(1ull << bit, false);
      stage = MAX2(stage, new_stage);
   }

   return stage;
}

static enum tu_stage
vk2tu_dst_stage(VkPipelineStageFlags2 vk_stages)
{
   enum tu_stage stage = TU_STAGE_BOTTOM;
   u_foreach_bit64 (bit, vk_stages) {
      enum tu_stage new_stage = vk2tu_single_stage(1ull << bit, true);
      stage = MIN2(stage, new_stage);
   }

   return stage;
}

static void
tu_flush_for_stage(struct tu_cache_state *cache,
                   enum tu_stage src_stage, enum tu_stage dst_stage)
{
   /* Even if the source is the host or CP, the destination access could
    * generate invalidates that we have to wait to complete.
    */
   if (src_stage == TU_STAGE_CP &&
       (cache->flush_bits & TU_CMD_FLAG_ALL_INVALIDATE))
      src_stage = TU_STAGE_GPU;

   if (src_stage >= dst_stage) {
      cache->flush_bits |= TU_CMD_FLAG_WAIT_FOR_IDLE;
      if (dst_stage == TU_STAGE_CP)
         cache->pending_flush_bits |= TU_CMD_FLAG_WAIT_FOR_ME;
   }
}

void
tu_render_pass_state_merge(struct tu_render_pass_state *dst,
                           const struct tu_render_pass_state *src)
{
   dst->xfb_used |= src->xfb_used;
   dst->has_tess |= src->has_tess;
   dst->has_prim_generated_query_in_rp |= src->has_prim_generated_query_in_rp;
   dst->disable_gmem |= src->disable_gmem;
   dst->sysmem_single_prim_mode |= src->sysmem_single_prim_mode;
   dst->draw_cs_writes_to_cond_pred |= src->draw_cs_writes_to_cond_pred;
   dst->shared_viewport |= src->shared_viewport;

   dst->drawcall_count += src->drawcall_count;
   dst->drawcall_bandwidth_per_sample_sum +=
      src->drawcall_bandwidth_per_sample_sum;
}

void
tu_restore_suspended_pass(struct tu_cmd_buffer *cmd,
                          struct tu_cmd_buffer *suspended)
{
   cmd->state.pass = suspended->state.suspended_pass.pass;
   cmd->state.subpass = suspended->state.suspended_pass.subpass;
   cmd->state.framebuffer = suspended->state.suspended_pass.framebuffer;
   cmd->state.attachments = suspended->state.suspended_pass.attachments;
   cmd->state.render_area = suspended->state.suspended_pass.render_area;
   cmd->state.gmem_layout = suspended->state.suspended_pass.gmem_layout;
   cmd->state.tiling = &cmd->state.framebuffer->tiling[cmd->state.gmem_layout];
   cmd->state.lrz = suspended->state.suspended_pass.lrz;
}

/* Take the saved pre-chain in "secondary" and copy its commands to "cmd",
 * appending it after any saved-up commands in "cmd".
 */
void
tu_append_pre_chain(struct tu_cmd_buffer *cmd,
                    struct tu_cmd_buffer *secondary)
{
   tu_cs_add_entries(&cmd->draw_cs, &secondary->pre_chain.draw_cs);
   tu_cs_add_entries(&cmd->draw_epilogue_cs,
                     &secondary->pre_chain.draw_epilogue_cs);

   tu_render_pass_state_merge(&cmd->state.rp,
                              &secondary->pre_chain.state);
   tu_clone_trace_range(cmd, &cmd->draw_cs, secondary->pre_chain.trace_renderpass_start,
         secondary->pre_chain.trace_renderpass_end);
   util_dynarray_append_dynarray(&cmd->fdm_bin_patchpoints,
                                 &secondary->pre_chain.fdm_bin_patchpoints);
}

/* Take the saved post-chain in "secondary" and copy it to "cmd".
 */
void
tu_append_post_chain(struct tu_cmd_buffer *cmd,
                     struct tu_cmd_buffer *secondary)
{
   tu_cs_add_entries(&cmd->draw_cs, &secondary->draw_cs);
   tu_cs_add_entries(&cmd->draw_epilogue_cs, &secondary->draw_epilogue_cs);

   tu_clone_trace_range(cmd, &cmd->draw_cs, secondary->trace_renderpass_start,
         secondary->trace_renderpass_end);
   cmd->state.rp = secondary->state.rp;
   util_dynarray_append_dynarray(&cmd->fdm_bin_patchpoints,
                                 &secondary->fdm_bin_patchpoints);
}

/* Assuming "secondary" is just a sequence of suspended and resuming passes,
 * copy its state to "cmd". This also works instead of tu_append_post_chain(),
 * but it's a bit slower because we don't assume that the chain begins in
 * "secondary" and therefore have to care about the command buffer's
 * renderpass state.
 */
void
tu_append_pre_post_chain(struct tu_cmd_buffer *cmd,
                         struct tu_cmd_buffer *secondary)
{
   tu_cs_add_entries(&cmd->draw_cs, &secondary->draw_cs);
   tu_cs_add_entries(&cmd->draw_epilogue_cs, &secondary->draw_epilogue_cs);

   tu_clone_trace_range(cmd, &cmd->draw_cs, secondary->trace_renderpass_start,
         secondary->trace_renderpass_end);
   tu_render_pass_state_merge(&cmd->state.rp,
                              &secondary->state.rp);
   util_dynarray_append_dynarray(&cmd->fdm_bin_patchpoints,
                                 &secondary->fdm_bin_patchpoints);
}

/* Take the current render pass state and save it to "pre_chain" to be
 * combined later.
 */
static void
tu_save_pre_chain(struct tu_cmd_buffer *cmd)
{
   tu_cs_add_entries(&cmd->pre_chain.draw_cs,
                     &cmd->draw_cs);
   tu_cs_add_entries(&cmd->pre_chain.draw_epilogue_cs,
                     &cmd->draw_epilogue_cs);
   cmd->pre_chain.trace_renderpass_start =
      cmd->trace_renderpass_start;
   cmd->pre_chain.trace_renderpass_end =
      cmd->trace_renderpass_end;
   cmd->pre_chain.state = cmd->state.rp;
   util_dynarray_append_dynarray(&cmd->pre_chain.fdm_bin_patchpoints,
                                 &cmd->fdm_bin_patchpoints);
   cmd->pre_chain.patchpoints_ctx = cmd->patchpoints_ctx;
   cmd->patchpoints_ctx = NULL;
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdExecuteCommands(VkCommandBuffer commandBuffer,
                      uint32_t commandBufferCount,
                      const VkCommandBuffer *pCmdBuffers)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   VkResult result;

   assert(commandBufferCount > 0);

   /* Emit any pending flushes. */
   if (cmd->state.pass) {
      tu_flush_all_pending(&cmd->state.renderpass_cache);
      TU_CALLX(cmd->device, tu_emit_cache_flush_renderpass)(cmd);
   } else {
      tu_flush_all_pending(&cmd->state.cache);
      TU_CALLX(cmd->device, tu_emit_cache_flush)(cmd);
   }

   for (uint32_t i = 0; i < commandBufferCount; i++) {
      TU_FROM_HANDLE(tu_cmd_buffer, secondary, pCmdBuffers[i]);

      if (secondary->usage_flags &
          VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
         assert(tu_cs_is_empty(&secondary->cs));

         result = tu_cs_add_entries(&cmd->draw_cs, &secondary->draw_cs);
         if (result != VK_SUCCESS) {
            vk_command_buffer_set_error(&cmd->vk, result);
            break;
         }

         result = tu_cs_add_entries(&cmd->draw_epilogue_cs,
               &secondary->draw_epilogue_cs);
         if (result != VK_SUCCESS) {
            vk_command_buffer_set_error(&cmd->vk, result);
            break;
         }

         /* If LRZ was made invalid in secondary - we should disable
          * LRZ retroactively for the whole renderpass.
          */
         if (!secondary->state.lrz.valid)
            cmd->state.lrz.valid = false;

         tu_clone_trace(cmd, &cmd->draw_cs, &secondary->trace);
         tu_render_pass_state_merge(&cmd->state.rp, &secondary->state.rp);
         util_dynarray_append_dynarray(&cmd->fdm_bin_patchpoints,
                                       &secondary->fdm_bin_patchpoints);
      } else {
         switch (secondary->state.suspend_resume) {
         case SR_NONE:
            assert(tu_cs_is_empty(&secondary->draw_cs));
            assert(tu_cs_is_empty(&secondary->draw_epilogue_cs));
            tu_cs_add_entries(&cmd->cs, &secondary->cs);
            tu_clone_trace(cmd, &cmd->cs, &secondary->trace);
            break;

         case SR_IN_PRE_CHAIN:
            /* cmd may be empty, which means that the chain begins before cmd
             * in which case we have to update its state.
             */
            if (cmd->state.suspend_resume == SR_NONE) {
               cmd->state.suspend_resume = SR_IN_PRE_CHAIN;
               cmd->trace_renderpass_start = u_trace_end_iterator(&cmd->trace);
            }

            /* The secondary is just a continuous suspend/resume chain so we
             * just have to append it to the the command buffer.
             */
            assert(tu_cs_is_empty(&secondary->cs));
            tu_append_pre_post_chain(cmd, secondary);
            break;

         case SR_AFTER_PRE_CHAIN:
         case SR_IN_CHAIN:
         case SR_IN_CHAIN_AFTER_PRE_CHAIN:
            if (secondary->state.suspend_resume == SR_AFTER_PRE_CHAIN ||
                secondary->state.suspend_resume == SR_IN_CHAIN_AFTER_PRE_CHAIN) {
               /* In thse cases there is a `pre_chain` in the secondary which
                * ends that we need to append to the primary.
                */

               if (cmd->state.suspend_resume == SR_NONE)
                  cmd->trace_renderpass_start = u_trace_end_iterator(&cmd->trace);

               tu_append_pre_chain(cmd, secondary);

               /* We're about to render, so we need to end the command stream
                * in case there were any extra commands generated by copying
                * the trace.
                */
               tu_cs_end(&cmd->draw_cs);
               tu_cs_end(&cmd->draw_epilogue_cs);

               switch (cmd->state.suspend_resume) {
               case SR_NONE:
               case SR_IN_PRE_CHAIN:
                  /* The renderpass chain ends in the secondary but isn't
                   * started in the primary, so we have to move the state to
                   * `pre_chain`.
                   */
                  cmd->trace_renderpass_end = u_trace_end_iterator(&cmd->trace);
                  tu_save_pre_chain(cmd);
                  cmd->state.suspend_resume = SR_AFTER_PRE_CHAIN;
                  break;
               case SR_IN_CHAIN:
               case SR_IN_CHAIN_AFTER_PRE_CHAIN:
                  /* The renderpass ends in the secondary and starts somewhere
                   * earlier in this primary. Since the last render pass in
                   * the chain is in the secondary, we are technically outside
                   * of a render pass.  Fix that here by reusing the dynamic
                   * render pass that was setup for the last suspended render
                   * pass before the secondary.
                   */
                  tu_restore_suspended_pass(cmd, cmd);

                  TU_CALLX(cmd->device, tu_cmd_render)(cmd);
                  if (cmd->state.suspend_resume == SR_IN_CHAIN)
                     cmd->state.suspend_resume = SR_NONE;
                  else
                     cmd->state.suspend_resume = SR_AFTER_PRE_CHAIN;
                  break;
               case SR_AFTER_PRE_CHAIN:
                  unreachable("resuming render pass is not preceded by suspending one");
               }

               tu_reset_render_pass(cmd);
            }

            tu_cs_add_entries(&cmd->cs, &secondary->cs);

            if (secondary->state.suspend_resume == SR_IN_CHAIN_AFTER_PRE_CHAIN ||
                secondary->state.suspend_resume == SR_IN_CHAIN) {
               /* The secondary ends in a "post-chain" (the opposite of a
                * pre-chain) that we need to copy into the current command
                * buffer.
                */
               cmd->trace_renderpass_start = u_trace_end_iterator(&cmd->trace);
               tu_append_post_chain(cmd, secondary);
               cmd->trace_renderpass_end = u_trace_end_iterator(&cmd->trace);
               cmd->state.suspended_pass = secondary->state.suspended_pass;

               switch (cmd->state.suspend_resume) {
               case SR_NONE:
                  cmd->state.suspend_resume = SR_IN_CHAIN;
                  break;
               case SR_AFTER_PRE_CHAIN:
                  cmd->state.suspend_resume = SR_IN_CHAIN_AFTER_PRE_CHAIN;
                  break;
               default:
                  unreachable("suspending render pass is followed by a not resuming one");
               }
            }
         }
      }

      cmd->state.index_size = secondary->state.index_size; /* for restart index update */
   }
   cmd->state.dirty = ~0u; /* TODO: set dirty only what needs to be */

   if (!cmd->state.lrz.gpu_dir_tracking && cmd->state.pass) {
      /* After a secondary command buffer is executed, LRZ is not valid
       * until it is cleared again.
       */
      cmd->state.lrz.valid = false;
   }

   /* After executing secondary command buffers, there may have been arbitrary
    * flushes executed, so when we encounter a pipeline barrier with a
    * srcMask, we have to assume that we need to invalidate. Therefore we need
    * to re-initialize the cache with all pending invalidate bits set.
    */
   if (cmd->state.pass) {
      tu_cache_init(&cmd->state.renderpass_cache);
   } else {
      tu_cache_init(&cmd->state.cache);
   }
}

static void
tu_subpass_barrier(struct tu_cmd_buffer *cmd_buffer,
                   const struct tu_subpass_barrier *barrier,
                   bool external)
{
   /* Note: we don't know until the end of the subpass whether we'll use
    * sysmem, so assume sysmem here to be safe.
    */
   struct tu_cache_state *cache =
      external ? &cmd_buffer->state.cache : &cmd_buffer->state.renderpass_cache;
   VkPipelineStageFlags2 src_stage_vk =
      sanitize_src_stage(barrier->src_stage_mask);
   VkPipelineStageFlags2 dst_stage_vk =
      sanitize_dst_stage(barrier->dst_stage_mask);
   BITMASK_ENUM(tu_cmd_access_mask) src_flags =
      vk2tu_access(barrier->src_access_mask, src_stage_vk, false, false);
   BITMASK_ENUM(tu_cmd_access_mask) dst_flags =
      vk2tu_access(barrier->dst_access_mask, dst_stage_vk, false, false);

   if (barrier->incoherent_ccu_color)
      src_flags |= TU_ACCESS_CCU_COLOR_INCOHERENT_WRITE;
   if (barrier->incoherent_ccu_depth)
      src_flags |= TU_ACCESS_CCU_DEPTH_INCOHERENT_WRITE;

   tu_flush_for_access(cache, src_flags, dst_flags);

   enum tu_stage src_stage = vk2tu_src_stage(src_stage_vk);
   enum tu_stage dst_stage = vk2tu_dst_stage(dst_stage_vk);
   tu_flush_for_stage(cache, src_stage, dst_stage);
}

template <chip CHIP>
static void
tu_emit_subpass_begin_gmem(struct tu_cmd_buffer *cmd)
{
   struct tu_cs *cs = &cmd->draw_cs;
   uint32_t subpass_idx = cmd->state.subpass - cmd->state.pass->subpasses;

   /* If we might choose to bin, then put the loads under a check for geometry
    * having been binned to this tile.  If we don't choose to bin in the end,
    * then we will have manually set those registers to say geometry is present.
    *
    * However, if the draw CS has a write to the condition for some other reason
    * (perf queries), then we can't do this optimization since the
    * start-of-the-CS geometry condition will have been overwritten.
    */
   bool cond_load_allowed = cmd->state.tiling->binning &&
                            cmd->state.pass->has_cond_load_store &&
                            !cmd->state.rp.draw_cs_writes_to_cond_pred;

   tu_cond_exec_start(cs, CP_COND_EXEC_0_RENDER_MODE_GMEM);

   /* Emit gmem loads that are first used in this subpass. */
   bool emitted_scissor = false;
   for (uint32_t i = 0; i < cmd->state.pass->attachment_count; ++i) {
      struct tu_render_pass_attachment *att = &cmd->state.pass->attachments[i];
      if ((att->load || att->load_stencil) && att->first_subpass_idx == subpass_idx) {
         if (!emitted_scissor) {
            tu6_emit_blit_scissor(cmd, cs, true);
            emitted_scissor = true;
         }
         tu_load_gmem_attachment<CHIP>(cmd, cs, i, cond_load_allowed, false);
      }
   }

   /* Emit gmem clears that are first used in this subpass. */
   emitted_scissor = false;
   for (uint32_t i = 0; i < cmd->state.pass->attachment_count; ++i) {
      struct tu_render_pass_attachment *att = &cmd->state.pass->attachments[i];
      if (att->clear_mask && att->first_subpass_idx == subpass_idx) {
         if (!emitted_scissor) {
            tu6_emit_blit_scissor(cmd, cs, false);
            emitted_scissor = true;
         }
         tu_clear_gmem_attachment<CHIP>(cmd, cs, i);
      }
   }

   tu_cond_exec_end(cs); /* CP_COND_EXEC_0_RENDER_MODE_GMEM */
}

/* Emits sysmem clears that are first used in this subpass. */
template <chip CHIP>
static void
tu_emit_subpass_begin_sysmem(struct tu_cmd_buffer *cmd)
{
   struct tu_cs *cs = &cmd->draw_cs;
   uint32_t subpass_idx = cmd->state.subpass - cmd->state.pass->subpasses;

   tu_cond_exec_start(cs, CP_COND_EXEC_0_RENDER_MODE_SYSMEM);
   for (uint32_t i = 0; i < cmd->state.pass->attachment_count; ++i) {
      struct tu_render_pass_attachment *att = &cmd->state.pass->attachments[i];
      if (att->clear_mask && att->first_subpass_idx == subpass_idx)
         tu_clear_sysmem_attachment<CHIP>(cmd, cs, i);
   }
   tu_cond_exec_end(cs); /* sysmem */
}

/* emit loads, clears, and mrt/zs/msaa/ubwc state for the subpass that is
 * starting (either at vkCmdBeginRenderPass2() or vkCmdNextSubpass2())
 *
 * Clears and loads have to happen at this point, because with
 * VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT the loads may depend on the output of
 * a previous aliased attachment's store.
 */
template <chip CHIP>
static void
tu_emit_subpass_begin(struct tu_cmd_buffer *cmd)
{
   tu_fill_render_pass_state(&cmd->state.vk_rp, cmd->state.pass, cmd->state.subpass);

   tu_emit_subpass_begin_gmem<CHIP>(cmd);
   tu_emit_subpass_begin_sysmem<CHIP>(cmd);

   tu6_emit_zs<CHIP>(cmd, cmd->state.subpass, &cmd->draw_cs);
   tu6_emit_mrt(cmd, cmd->state.subpass, &cmd->draw_cs);
   tu6_emit_render_cntl<CHIP>(cmd, cmd->state.subpass, &cmd->draw_cs, false);

   tu_set_input_attachments(cmd, cmd->state.subpass);

   vk_cmd_set_cb_attachment_count(&cmd->vk, cmd->state.subpass->color_count);

   cmd->state.dirty |= TU_CMD_DIRTY_SUBPASS;
}

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                       const VkRenderPassBeginInfo *pRenderPassBegin,
                       const VkSubpassBeginInfo *pSubpassBeginInfo)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);

   if (TU_DEBUG(DYNAMIC)) {
      vk_common_CmdBeginRenderPass2(commandBuffer, pRenderPassBegin,
                                    pSubpassBeginInfo);
      return;
   }

   TU_FROM_HANDLE(tu_render_pass, pass, pRenderPassBegin->renderPass);
   TU_FROM_HANDLE(tu_framebuffer, fb, pRenderPassBegin->framebuffer);

   const struct VkRenderPassAttachmentBeginInfo *pAttachmentInfo =
      vk_find_struct_const(pRenderPassBegin->pNext,
                           RENDER_PASS_ATTACHMENT_BEGIN_INFO);

   cmd->state.pass = pass;
   cmd->state.subpass = pass->subpasses;
   cmd->state.framebuffer = fb;
   cmd->state.render_area = pRenderPassBegin->renderArea;

   VK_MULTIALLOC(ma);
   vk_multialloc_add(&ma, &cmd->state.attachments,
                     const struct tu_image_view *, pass->attachment_count);
   vk_multialloc_add(&ma, &cmd->state.clear_values, VkClearValue,
                     pRenderPassBegin->clearValueCount);
   if (!vk_multialloc_alloc(&ma, &cmd->vk.pool->alloc,
                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT)) {
      vk_command_buffer_set_error(&cmd->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return;
   }

   if (cmd->device->dbg_renderpass_stomp_cs) {
      tu_cs_emit_call(&cmd->cs, cmd->device->dbg_renderpass_stomp_cs);
   }

   for (unsigned i = 0; i < pass->attachment_count; i++) {
      cmd->state.attachments[i] = pAttachmentInfo ?
         tu_image_view_from_handle(pAttachmentInfo->pAttachments[i]) :
         cmd->state.framebuffer->attachments[i].attachment;
   }
   for (unsigned i = 0; i < pRenderPassBegin->clearValueCount; i++)
         cmd->state.clear_values[i] = pRenderPassBegin->pClearValues[i];

   tu_choose_gmem_layout(cmd);

   trace_start_render_pass(&cmd->trace, &cmd->cs, cmd->state.framebuffer,
                           cmd->state.tiling);

   /* Note: because this is external, any flushes will happen before draw_cs
    * gets called. However deferred flushes could have to happen later as part
    * of the subpass.
    */
   tu_subpass_barrier(cmd, &pass->subpasses[0].start_barrier, true);
   cmd->state.renderpass_cache.pending_flush_bits =
      cmd->state.cache.pending_flush_bits;
   cmd->state.renderpass_cache.flush_bits = 0;

   if (pass->subpasses[0].feedback_invalidate)
      cmd->state.renderpass_cache.flush_bits |= TU_CMD_FLAG_CACHE_INVALIDATE;

   tu_lrz_begin_renderpass(cmd);

   cmd->trace_renderpass_start = u_trace_end_iterator(&cmd->trace);

   tu_emit_renderpass_begin(cmd);
   tu_emit_subpass_begin<CHIP>(cmd);

   if (pass->has_fdm)
      cmd->patchpoints_ctx = ralloc_parent(NULL);
}
TU_GENX(tu_CmdBeginRenderPass2);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdBeginRendering(VkCommandBuffer commandBuffer,
                     const VkRenderingInfo *pRenderingInfo)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);

   tu_setup_dynamic_render_pass(cmd, pRenderingInfo);
   tu_setup_dynamic_framebuffer(cmd, pRenderingInfo);

   cmd->state.pass = &cmd->dynamic_pass;
   cmd->state.subpass = &cmd->dynamic_subpass;
   cmd->state.framebuffer = &cmd->dynamic_framebuffer;
   cmd->state.render_area = pRenderingInfo->renderArea;

   cmd->state.attachments = cmd->dynamic_attachments;
   cmd->state.clear_values = cmd->dynamic_clear_values;

   for (unsigned i = 0; i < pRenderingInfo->colorAttachmentCount; i++) {
      uint32_t a = cmd->dynamic_subpass.color_attachments[i].attachment;
      if (!pRenderingInfo->pColorAttachments[i].imageView)
         continue;

      cmd->state.clear_values[a] =
         pRenderingInfo->pColorAttachments[i].clearValue;

      TU_FROM_HANDLE(tu_image_view, view,
                     pRenderingInfo->pColorAttachments[i].imageView);
      cmd->state.attachments[a] = view;

      a = cmd->dynamic_subpass.resolve_attachments[i].attachment;
      if (a != VK_ATTACHMENT_UNUSED) {
         TU_FROM_HANDLE(tu_image_view, resolve_view,
                        pRenderingInfo->pColorAttachments[i].resolveImageView);
         cmd->state.attachments[a] = resolve_view;
      }
   }

   uint32_t a = cmd->dynamic_subpass.depth_stencil_attachment.attachment;
   if (pRenderingInfo->pDepthAttachment || pRenderingInfo->pStencilAttachment) {
      const struct VkRenderingAttachmentInfo *common_info =
         (pRenderingInfo->pDepthAttachment &&
          pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) ?
         pRenderingInfo->pDepthAttachment :
         pRenderingInfo->pStencilAttachment;
      if (common_info && common_info->imageView != VK_NULL_HANDLE) {
         TU_FROM_HANDLE(tu_image_view, view, common_info->imageView);
         cmd->state.attachments[a] = view;
         if (pRenderingInfo->pDepthAttachment) {
            cmd->state.clear_values[a].depthStencil.depth =
               pRenderingInfo->pDepthAttachment->clearValue.depthStencil.depth;
         }

         if (pRenderingInfo->pStencilAttachment) {
            cmd->state.clear_values[a].depthStencil.stencil =
               pRenderingInfo->pStencilAttachment->clearValue.depthStencil.stencil;
         }

         if (cmd->dynamic_subpass.resolve_count >
             cmd->dynamic_subpass.color_count) {
            TU_FROM_HANDLE(tu_image_view, resolve_view,
                           common_info->resolveImageView);
            a = cmd->dynamic_subpass.resolve_attachments[cmd->dynamic_subpass.color_count].attachment;
            cmd->state.attachments[a] = resolve_view;
         }
      }
   }

   a = cmd->dynamic_pass.fragment_density_map.attachment;
   if (a != VK_ATTACHMENT_UNUSED) {
      const VkRenderingFragmentDensityMapAttachmentInfoEXT *fdm_info =
         vk_find_struct_const(pRenderingInfo->pNext,
                              RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT);
      TU_FROM_HANDLE(tu_image_view, view, fdm_info->imageView);
      cmd->state.attachments[a] = view;
   }

   if (cmd->dynamic_pass.has_fdm)
      cmd->patchpoints_ctx = ralloc_context(NULL);

   tu_choose_gmem_layout(cmd);

   cmd->state.renderpass_cache.pending_flush_bits =
      cmd->state.cache.pending_flush_bits;
   cmd->state.renderpass_cache.flush_bits = 0;

   bool resuming = pRenderingInfo->flags & VK_RENDERING_RESUMING_BIT;
   bool suspending = pRenderingInfo->flags & VK_RENDERING_SUSPENDING_BIT;
   cmd->state.suspending = suspending;
   cmd->state.resuming = resuming;

   if (!resuming && cmd->device->dbg_renderpass_stomp_cs) {
      tu_cs_emit_call(&cmd->cs, cmd->device->dbg_renderpass_stomp_cs);
   }

   /* We can't track LRZ across command buffer boundaries, so we have to
    * disable LRZ when resuming/suspending unless we can track on the GPU.
    */
   if ((resuming || suspending) &&
       !cmd->device->physical_device->info->a6xx.has_lrz_dir_tracking) {
      cmd->state.lrz.valid = false;
   } else {
      if (resuming)
         tu_lrz_begin_resumed_renderpass(cmd);
      else
         tu_lrz_begin_renderpass(cmd);
   }


   if (suspending) {
      cmd->state.suspended_pass.pass = cmd->state.pass;
      cmd->state.suspended_pass.subpass = cmd->state.subpass;
      cmd->state.suspended_pass.framebuffer = cmd->state.framebuffer;
      cmd->state.suspended_pass.render_area = cmd->state.render_area;
      cmd->state.suspended_pass.attachments = cmd->state.attachments;
      cmd->state.suspended_pass.gmem_layout = cmd->state.gmem_layout;
   }

   if (!resuming)
      trace_start_render_pass(&cmd->trace, &cmd->cs, cmd->state.framebuffer,
                              cmd->state.tiling);

   if (!resuming || cmd->state.suspend_resume == SR_NONE) {
      cmd->trace_renderpass_start = u_trace_end_iterator(&cmd->trace);
   }

   if (!resuming) {
      tu_emit_renderpass_begin(cmd);
      tu_emit_subpass_begin<CHIP>(cmd);
   }

   if (suspending && !resuming) {
      /* entering a chain */
      switch (cmd->state.suspend_resume) {
      case SR_NONE:
         cmd->state.suspend_resume = SR_IN_CHAIN;
         break;
      case SR_AFTER_PRE_CHAIN:
         cmd->state.suspend_resume = SR_IN_CHAIN_AFTER_PRE_CHAIN;
         break;
      case SR_IN_PRE_CHAIN:
      case SR_IN_CHAIN:
      case SR_IN_CHAIN_AFTER_PRE_CHAIN:
         unreachable("suspending render pass not followed by resuming pass");
         break;
      }
   }

   if (resuming && cmd->state.suspend_resume == SR_NONE)
      cmd->state.suspend_resume = SR_IN_PRE_CHAIN;
}
TU_GENX(tu_CmdBeginRendering);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdNextSubpass2(VkCommandBuffer commandBuffer,
                   const VkSubpassBeginInfo *pSubpassBeginInfo,
                   const VkSubpassEndInfo *pSubpassEndInfo)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);

   if (TU_DEBUG(DYNAMIC)) {
      vk_common_CmdNextSubpass2(commandBuffer, pSubpassBeginInfo,
                                pSubpassEndInfo);
      return;
   }

   const struct tu_render_pass *pass = cmd->state.pass;
   const struct tu_framebuffer *fb = cmd->state.framebuffer;
   struct tu_cs *cs = &cmd->draw_cs;
   const struct tu_subpass *last_subpass = cmd->state.subpass;

   const struct tu_subpass *subpass = cmd->state.subpass++;

   /* Track LRZ valid state
    *
    * TODO: Improve this tracking for keeping the state of the past depth/stencil images,
    * so if they become active again, we reuse its old state.
    */
   if (last_subpass->depth_stencil_attachment.attachment != subpass->depth_stencil_attachment.attachment) {
      cmd->state.lrz.valid = false;
      cmd->state.dirty |= TU_CMD_DIRTY_LRZ;
   }

   if (cmd->state.tiling->possible) {
      if (cmd->state.pass->has_fdm)
         tu_cs_set_writeable(cs, true);

      tu_cond_exec_start(cs, CP_COND_EXEC_0_RENDER_MODE_GMEM);

      if (subpass->resolve_attachments) {
         tu6_emit_blit_scissor(cmd, cs, true);

         for (unsigned i = 0; i < subpass->resolve_count; i++) {
            uint32_t a = subpass->resolve_attachments[i].attachment;
            if (a == VK_ATTACHMENT_UNUSED)
               continue;

            uint32_t gmem_a = tu_subpass_get_attachment_to_resolve(subpass, i);

            tu_store_gmem_attachment<CHIP>(cmd, cs, a, gmem_a, fb->layers,
                                    subpass->multiview_mask, false);

            if (!pass->attachments[a].gmem)
               continue;

            /* check if the resolved attachment is needed by later subpasses,
            * if it is, should be doing a GMEM->GMEM resolve instead of GMEM->MEM->GMEM..
            */
            perf_debug(cmd->device, "TODO: missing GMEM->GMEM resolve path\n");
            tu_load_gmem_attachment<CHIP>(cmd, cs, a, false, true);
         }
      }

      tu_cond_exec_end(cs);

      if (cmd->state.pass->has_fdm)
         tu_cs_set_writeable(cs, false);

      tu_cond_exec_start(cs, CP_COND_EXEC_0_RENDER_MODE_SYSMEM);
   }

   tu6_emit_sysmem_resolves<CHIP>(cmd, cs, subpass);

   if (cmd->state.tiling->possible)
      tu_cond_exec_end(cs);

   /* Handle dependencies for the next subpass */
   tu_subpass_barrier(cmd, &cmd->state.subpass->start_barrier, false);

   if (cmd->state.subpass->feedback_invalidate)
      cmd->state.renderpass_cache.flush_bits |= TU_CMD_FLAG_CACHE_INVALIDATE;

   tu_emit_subpass_begin<CHIP>(cmd);
}
TU_GENX(tu_CmdNextSubpass2);

static uint32_t
tu6_user_consts_size(const struct tu_const_state *const_state,
                     gl_shader_stage type)
{
   uint32_t dwords = 0;

   if (const_state->push_consts.type == IR3_PUSH_CONSTS_PER_STAGE) {
      unsigned num_units = const_state->push_consts.dwords;
      dwords += 4 + num_units;
      assert(num_units > 0);
   }

   dwords += 8 * const_state->num_inline_ubos;

   return dwords;
}

static void
tu6_emit_per_stage_push_consts(struct tu_cs *cs,
                               const struct tu_const_state *const_state,
                               gl_shader_stage type,
                               uint32_t *push_constants)
{
   if (const_state->push_consts.type == IR3_PUSH_CONSTS_PER_STAGE) {
      unsigned num_units = const_state->push_consts.dwords;
      unsigned offset = const_state->push_consts.lo;
      assert(num_units > 0);

      /* DST_OFF and NUM_UNIT requires vec4 units */
      tu_cs_emit_pkt7(cs, tu6_stage2opcode(type), 3 + num_units);
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(offset / 4) |
            CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
            CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
            CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(type)) |
            CP_LOAD_STATE6_0_NUM_UNIT(num_units / 4));
      tu_cs_emit(cs, 0);
      tu_cs_emit(cs, 0);
      for (unsigned i = 0; i < num_units; i++)
         tu_cs_emit(cs, push_constants[i + offset]);
   }
}

static void
tu6_emit_inline_ubo(struct tu_cs *cs,
                    const struct tu_const_state *const_state,
                    unsigned constlen,
                    gl_shader_stage type,
                    struct tu_descriptor_state *descriptors)
{
   /* Emit loads of inline uniforms. These load directly from the uniform's
    * storage space inside the descriptor set.
    */
   for (unsigned i = 0; i < const_state->num_inline_ubos; i++) {
      const struct tu_inline_ubo *ubo = &const_state->ubos[i];

      if (constlen <= ubo->const_offset_vec4)
         continue;

      uint64_t va = descriptors->set_iova[ubo->base] & ~0x3f;

      tu_cs_emit_pkt7(cs, tu6_stage2opcode(type), ubo->push_address ? 7 : 3);
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(ubo->const_offset_vec4) |
            CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
            CP_LOAD_STATE6_0_STATE_SRC(ubo->push_address ? SS6_DIRECT : SS6_INDIRECT) |
            CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(type)) |
            CP_LOAD_STATE6_0_NUM_UNIT(MIN2(ubo->size_vec4, constlen - ubo->const_offset_vec4)));
      if (ubo->push_address) {
         tu_cs_emit(cs, 0);
         tu_cs_emit(cs, 0);
         tu_cs_emit_qw(cs, va + ubo->offset);
         tu_cs_emit(cs, 0);
         tu_cs_emit(cs, 0);
      } else {
         tu_cs_emit_qw(cs, va + ubo->offset);
      }
   }
}

static void
tu6_emit_shared_consts(struct tu_cs *cs,
                       const struct tu_push_constant_range *shared_consts,
                       uint32_t *push_constants,
                       bool compute)
{
   if (shared_consts->dwords > 0) {
      /* Offset and num_units for shared consts are in units of dwords. */
      unsigned num_units = shared_consts->dwords;
      unsigned offset = shared_consts->lo;

      enum a6xx_state_type st = compute ? ST6_UBO : ST6_CONSTANTS;
      uint32_t cp_load_state = compute ? CP_LOAD_STATE6_FRAG : CP_LOAD_STATE6;

      tu_cs_emit_pkt7(cs, cp_load_state, 3 + num_units);
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(offset) |
            CP_LOAD_STATE6_0_STATE_TYPE(st) |
            CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
            CP_LOAD_STATE6_0_STATE_BLOCK(SB6_IBO) |
            CP_LOAD_STATE6_0_NUM_UNIT(num_units));
      tu_cs_emit(cs, 0);
      tu_cs_emit(cs, 0);

      for (unsigned i = 0; i < num_units; i++)
         tu_cs_emit(cs, push_constants[i + offset]);
   }
}

static void
tu7_emit_shared_preamble_consts(
   struct tu_cs *cs,
   const struct tu_push_constant_range *shared_consts,
   uint32_t *push_constants)
{
   tu_cs_emit_pkt4(cs, REG_A7XX_HLSQ_SHARED_CONSTS_IMM(shared_consts->lo),
                   shared_consts->dwords);
   tu_cs_emit_array(cs, push_constants + shared_consts->lo,
                    shared_consts->dwords);
}

static uint32_t
tu6_const_size(struct tu_cmd_buffer *cmd,
               const struct tu_push_constant_range *shared_consts,
               bool compute)
{
   uint32_t dwords = 0;

   if (shared_consts->type == IR3_PUSH_CONSTS_SHARED) {
      dwords += shared_consts->dwords + 4;
   } else if (shared_consts->type == IR3_PUSH_CONSTS_SHARED_PREAMBLE) {
      dwords += shared_consts->dwords + 1;
   }

   if (compute) {
      dwords +=
         tu6_user_consts_size(&cmd->state.shaders[MESA_SHADER_COMPUTE]->const_state, MESA_SHADER_COMPUTE);
   } else {
      for (uint32_t type = MESA_SHADER_VERTEX; type <= MESA_SHADER_FRAGMENT; type++)
         dwords += tu6_user_consts_size(&cmd->state.shaders[type]->const_state, (gl_shader_stage) type);
   }

   return dwords;
}

static struct tu_draw_state
tu_emit_consts(struct tu_cmd_buffer *cmd, bool compute)
{
   uint32_t dwords = 0;
   const struct tu_push_constant_range *shared_consts =
      compute ? &cmd->state.shaders[MESA_SHADER_COMPUTE]->const_state.push_consts :
      &cmd->state.program.shared_consts;

   dwords = tu6_const_size(cmd, shared_consts, compute);

   if (dwords == 0)
      return (struct tu_draw_state) {};

   struct tu_cs cs;
   tu_cs_begin_sub_stream(&cmd->sub_cs, dwords, &cs);

   if (shared_consts->type == IR3_PUSH_CONSTS_SHARED) {
      tu6_emit_shared_consts(&cs, shared_consts, cmd->push_constants, compute);
   } else if (shared_consts->type == IR3_PUSH_CONSTS_SHARED_PREAMBLE) {
      tu7_emit_shared_preamble_consts(&cs, shared_consts, cmd->push_constants);
   }

   if (compute) {
      tu6_emit_per_stage_push_consts(
         &cs, &cmd->state.shaders[MESA_SHADER_COMPUTE]->const_state,
         MESA_SHADER_COMPUTE, cmd->push_constants);
      tu6_emit_inline_ubo(
         &cs, &cmd->state.shaders[MESA_SHADER_COMPUTE]->const_state,
         cmd->state.shaders[MESA_SHADER_COMPUTE]->variant->constlen,
         MESA_SHADER_COMPUTE,
         tu_get_descriptors_state(cmd, VK_PIPELINE_BIND_POINT_COMPUTE));
   } else {
      struct tu_descriptor_state *descriptors =
         tu_get_descriptors_state(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS);
      for (uint32_t type = MESA_SHADER_VERTEX; type <= MESA_SHADER_FRAGMENT; type++) {
         const struct tu_program_descriptor_linkage *link =
            &cmd->state.program.link[type];
         tu6_emit_per_stage_push_consts(&cs, &link->tu_const_state,
                                        (gl_shader_stage) type,
                                        cmd->push_constants);
         tu6_emit_inline_ubo(&cs, &link->tu_const_state, link->constlen,
                             (gl_shader_stage) type, descriptors);
      }
   }

   return tu_cs_end_draw_state(&cmd->sub_cs, &cs);
}

/* Various frontends (ANGLE, zink at least) will enable stencil testing with
 * what works out to be no-op writes.  Simplify what they give us into flags
 * that LRZ can use.
 */
static void
tu6_update_simplified_stencil_state(struct tu_cmd_buffer *cmd)
{
   const struct vk_depth_stencil_state *ds =
      &cmd->vk.dynamic_graphics_state.ds;
   bool stencil_test_enable = ds->stencil.test_enable;

   if (!stencil_test_enable) {
      cmd->state.stencil_front_write = false;
      cmd->state.stencil_back_write = false;
      return;
   }

   bool stencil_front_writemask = ds->stencil.front.write_mask;
   bool stencil_back_writemask = ds->stencil.back.write_mask;

   VkStencilOp front_fail_op = (VkStencilOp)ds->stencil.front.op.fail;
   VkStencilOp front_pass_op = (VkStencilOp)ds->stencil.front.op.pass;
   VkStencilOp front_depth_fail_op = (VkStencilOp)ds->stencil.front.op.depth_fail;
   VkStencilOp back_fail_op = (VkStencilOp)ds->stencil.back.op.fail;
   VkStencilOp back_pass_op = (VkStencilOp)ds->stencil.back.op.pass;
   VkStencilOp back_depth_fail_op = (VkStencilOp)ds->stencil.back.op.depth_fail;

   bool stencil_front_op_writes =
      front_pass_op != VK_STENCIL_OP_KEEP ||
      front_fail_op != VK_STENCIL_OP_KEEP ||
      front_depth_fail_op != VK_STENCIL_OP_KEEP;

   bool stencil_back_op_writes =
      back_pass_op != VK_STENCIL_OP_KEEP ||
      back_fail_op != VK_STENCIL_OP_KEEP ||
      back_depth_fail_op != VK_STENCIL_OP_KEEP;

   cmd->state.stencil_front_write =
      stencil_front_op_writes && stencil_front_writemask;
   cmd->state.stencil_back_write =
      stencil_back_op_writes && stencil_back_writemask;
}

static bool
tu6_writes_depth(struct tu_cmd_buffer *cmd, bool depth_test_enable)
{
   bool depth_write_enable =
      cmd->vk.dynamic_graphics_state.ds.depth.write_enable;

   VkCompareOp depth_compare_op = (VkCompareOp)
      cmd->vk.dynamic_graphics_state.ds.depth.compare_op;

   bool depth_compare_op_writes = depth_compare_op != VK_COMPARE_OP_NEVER;

   return depth_test_enable && depth_write_enable && depth_compare_op_writes;
}

static bool
tu6_writes_stencil(struct tu_cmd_buffer *cmd)
{
   return cmd->state.stencil_front_write || cmd->state.stencil_back_write;
}

static void
tu6_build_depth_plane_z_mode(struct tu_cmd_buffer *cmd, struct tu_cs *cs)
{
   enum a6xx_ztest_mode zmode = A6XX_EARLY_Z;
   bool depth_test_enable = cmd->vk.dynamic_graphics_state.ds.depth.test_enable;
   bool depth_write = tu6_writes_depth(cmd, depth_test_enable);
   bool stencil_write = tu6_writes_stencil(cmd);
   const struct tu_shader *fs = cmd->state.shaders[MESA_SHADER_FRAGMENT];
   const struct tu_render_pass *pass = cmd->state.pass;
   const struct tu_subpass *subpass = cmd->state.subpass;

   if ((fs->variant->has_kill ||
        cmd->state.pipeline_feedback_loop_ds) &&
       (depth_write || stencil_write)) {
      zmode = (cmd->state.lrz.valid && cmd->state.lrz.enabled)
                 ? A6XX_EARLY_LRZ_LATE_Z
                 : A6XX_LATE_Z;
   }

   bool force_late_z = 
      (subpass->depth_stencil_attachment.attachment != VK_ATTACHMENT_UNUSED &&
       pass->attachments[subpass->depth_stencil_attachment.attachment].format
       == VK_FORMAT_S8_UINT) ||
      fs->fs.lrz.force_late_z ||
      /* alpha-to-coverage can behave like a discard. */
      cmd->vk.dynamic_graphics_state.ms.alpha_to_coverage_enable;
   if ((force_late_z && !fs->variant->fs.early_fragment_tests) ||
       !depth_test_enable)
      zmode = A6XX_LATE_Z;

   /* User defined early tests take precedence above all else */
   if (fs->variant->fs.early_fragment_tests)
      zmode = A6XX_EARLY_Z;

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_SU_DEPTH_PLANE_CNTL, 1);
   tu_cs_emit(cs, A6XX_GRAS_SU_DEPTH_PLANE_CNTL_Z_MODE(zmode));

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_DEPTH_PLANE_CNTL, 1);
   tu_cs_emit(cs, A6XX_RB_DEPTH_PLANE_CNTL_Z_MODE(zmode));
}

static uint32_t
fs_params_offset(struct tu_cmd_buffer *cmd)
{
   const struct tu_program_descriptor_linkage *link =
      &cmd->state.program.link[MESA_SHADER_FRAGMENT];
   const struct ir3_const_state *const_state = &link->const_state;

   if (const_state->num_driver_params <= IR3_DP_FS_DYNAMIC)
      return 0;

   if (const_state->offsets.driver_param + IR3_DP_FS_DYNAMIC / 4 >= link->constlen)
      return 0;

   return const_state->offsets.driver_param + IR3_DP_FS_DYNAMIC / 4;
}

static uint32_t
fs_params_size(struct tu_cmd_buffer *cmd)
{
   const struct tu_program_descriptor_linkage *link =
      &cmd->state.program.link[MESA_SHADER_FRAGMENT];
   const struct ir3_const_state *const_state = &link->const_state;

   return DIV_ROUND_UP(const_state->num_driver_params - IR3_DP_FS_DYNAMIC, 4);
}

struct apply_fs_params_state {
   unsigned num_consts;
};

static void
fdm_apply_fs_params(struct tu_cs *cs, void *data, VkRect2D bin, unsigned views,
                    VkExtent2D *frag_areas)
{
   const struct apply_fs_params_state *state =
      (const struct apply_fs_params_state *)data;
   unsigned num_consts = state->num_consts;

   for (unsigned i = 0; i < num_consts; i++) {
      assert(i < views);
      VkExtent2D area = frag_areas[i];
      VkOffset2D offset = tu_fdm_per_bin_offset(area, bin);
      
      tu_cs_emit(cs, area.width);
      tu_cs_emit(cs, area.height);
      tu_cs_emit(cs, fui(offset.x));
      tu_cs_emit(cs, fui(offset.y));
   }
}

static void
tu6_emit_fs_params(struct tu_cmd_buffer *cmd)
{
   uint32_t offset = fs_params_offset(cmd);

   if (offset == 0) {
      cmd->state.fs_params = (struct tu_draw_state) {};
      return;
   }

   struct tu_shader *fs = cmd->state.shaders[MESA_SHADER_FRAGMENT];

   unsigned num_units = fs_params_size(cmd);

   if (fs->fs.has_fdm)
      tu_cs_set_writeable(&cmd->sub_cs, true);

   struct tu_cs cs;
   VkResult result = tu_cs_begin_sub_stream(&cmd->sub_cs, 4 + 4 * num_units, &cs);
   if (result != VK_SUCCESS) {
      tu_cs_set_writeable(&cmd->sub_cs, false);
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   tu_cs_emit_pkt7(&cs, CP_LOAD_STATE6_FRAG, 3 + 4 * num_units);
   tu_cs_emit(&cs, CP_LOAD_STATE6_0_DST_OFF(offset) |
         CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
         CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
         CP_LOAD_STATE6_0_STATE_BLOCK(SB6_FS_SHADER) |
         CP_LOAD_STATE6_0_NUM_UNIT(num_units));
   tu_cs_emit(&cs, 0);
   tu_cs_emit(&cs, 0);

   STATIC_ASSERT(IR3_DP_FS_FRAG_INVOCATION_COUNT == IR3_DP_FS_DYNAMIC);
   tu_cs_emit(&cs, fs->fs.per_samp ?
              cmd->vk.dynamic_graphics_state.ms.rasterization_samples : 1);
   tu_cs_emit(&cs, 0);
   tu_cs_emit(&cs, 0);
   tu_cs_emit(&cs, 0);

   STATIC_ASSERT(IR3_DP_FS_FRAG_SIZE == IR3_DP_FS_DYNAMIC + 4);
   STATIC_ASSERT(IR3_DP_FS_FRAG_OFFSET == IR3_DP_FS_DYNAMIC + 6);
   if (num_units > 1) {
      if (fs->fs.has_fdm) {
         struct apply_fs_params_state state = {
            .num_consts = num_units - 1,
         };
         tu_create_fdm_bin_patchpoint(cmd, &cs, 4 * (num_units - 1),
                                      fdm_apply_fs_params, state);
      } else {
         for (unsigned i = 1; i < num_units; i++) {
            tu_cs_emit(&cs, 1);
            tu_cs_emit(&cs, 1);
            tu_cs_emit(&cs, fui(0.0f));
            tu_cs_emit(&cs, fui(0.0f));
         }
      }
   }

   cmd->state.fs_params = tu_cs_end_draw_state(&cmd->sub_cs, &cs);

   if (fs->fs.has_fdm)
      tu_cs_set_writeable(&cmd->sub_cs, false);
}

template <chip CHIP>
static VkResult
tu6_draw_common(struct tu_cmd_buffer *cmd,
                struct tu_cs *cs,
                bool indexed,
                /* note: draw_count is 0 for indirect */
                uint32_t draw_count)
{
   const struct tu_program_state *program = &cmd->state.program;
   struct tu_render_pass_state *rp = &cmd->state.rp;

   /* Emit state first, because it's needed for bandwidth calculations */
   uint32_t dynamic_draw_state_dirty = 0;
   if (!BITSET_IS_EMPTY(cmd->vk.dynamic_graphics_state.dirty) ||
       (cmd->state.dirty & ~TU_CMD_DIRTY_COMPUTE_DESC_SETS)) {
      dynamic_draw_state_dirty = tu_emit_draw_state<CHIP>(cmd);
   }

   /* Fill draw stats for autotuner */
   rp->drawcall_count++;

   rp->drawcall_bandwidth_per_sample_sum +=
      cmd->state.bandwidth.color_bandwidth_per_sample;

   /* add depth memory bandwidth cost */
   const uint32_t depth_bandwidth = cmd->state.bandwidth.depth_cpp_per_sample;
   if (cmd->vk.dynamic_graphics_state.ds.depth.write_enable)
      rp->drawcall_bandwidth_per_sample_sum += depth_bandwidth;
   if (cmd->vk.dynamic_graphics_state.ds.depth.test_enable)
      rp->drawcall_bandwidth_per_sample_sum += depth_bandwidth;

   /* add stencil memory bandwidth cost */
   const uint32_t stencil_bandwidth =
      cmd->state.bandwidth.stencil_cpp_per_sample;
   if (cmd->vk.dynamic_graphics_state.ds.stencil.test_enable)
      rp->drawcall_bandwidth_per_sample_sum += stencil_bandwidth * 2;

   tu_emit_cache_flush_renderpass<CHIP>(cmd);

  if (BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_RS_PROVOKING_VERTEX)) {
      bool primitive_restart_enabled =
         cmd->vk.dynamic_graphics_state.ia.primitive_restart_enable;

      bool primitive_restart = primitive_restart_enabled && indexed;
      bool provoking_vtx_last =
         cmd->vk.dynamic_graphics_state.rs.provoking_vertex ==
         VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT;

      uint32_t primitive_cntl_0 =
         A6XX_PC_PRIMITIVE_CNTL_0(.primitive_restart = primitive_restart,
                                  .provoking_vtx_last = provoking_vtx_last).value;
      tu_cs_emit_regs(cs, A6XX_PC_PRIMITIVE_CNTL_0(.dword = primitive_cntl_0));
      if (CHIP == A7XX) {
         tu_cs_emit_regs(cs, A7XX_VPC_PRIMITIVE_CNTL_0(.dword = primitive_cntl_0));
      }
   }

   struct tu_tess_params *tess_params = &cmd->state.tess_params;
   if ((cmd->state.dirty & TU_CMD_DIRTY_TESS_PARAMS) ||
       BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                   MESA_VK_DYNAMIC_TS_DOMAIN_ORIGIN)) {
      bool tess_upper_left_domain_origin =
         (VkTessellationDomainOrigin)cmd->vk.dynamic_graphics_state.ts.domain_origin ==
         VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT;
      tu_cs_emit_regs(cs, A6XX_PC_TESS_CNTL(
            .spacing = tess_params->spacing,
            .output = tess_upper_left_domain_origin ?
               tess_params->output_upper_left :
               tess_params->output_lower_left));
   }

   /* Early exit if there is nothing to emit, saves CPU cycles */
   uint32_t dirty = cmd->state.dirty;
   if (!dynamic_draw_state_dirty && !(dirty & ~TU_CMD_DIRTY_COMPUTE_DESC_SETS))
      return VK_SUCCESS;

   bool dirty_lrz =
      (dirty & TU_CMD_DIRTY_LRZ) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_ENABLE) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_DS_STENCIL_OP) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK) ||
      BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                  MESA_VK_DYNAMIC_MS_ALPHA_TO_COVERAGE_ENABLE);

   if (dirty_lrz) {
      struct tu_cs cs;
      uint32_t size = cmd->device->physical_device->info->a6xx.lrz_track_quirk ? 10 : 8;

      cmd->state.lrz_and_depth_plane_state =
         tu_cs_draw_state(&cmd->sub_cs, &cs, size);
      tu6_update_simplified_stencil_state(cmd);
      tu6_emit_lrz(cmd, &cs);
      tu6_build_depth_plane_z_mode(cmd, &cs);
   }

   if (BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                   MESA_VK_DYNAMIC_VI_BINDINGS_VALID)) {
      cmd->state.vertex_buffers.size =
         util_last_bit(cmd->vk.dynamic_graphics_state.vi_bindings_valid) * 4;
      cmd->state.dirty |= TU_CMD_DIRTY_VERTEX_BUFFERS;
   }

   if (dirty & TU_CMD_DIRTY_SHADER_CONSTS)
      cmd->state.shader_const = tu_emit_consts(cmd, false);

   if (dirty & TU_CMD_DIRTY_DESC_SETS)
      tu6_emit_descriptor_sets<CHIP>(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS);

   if (BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                   MESA_VK_DYNAMIC_MS_RASTERIZATION_SAMPLES) ||
       BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                   MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY) ||
       BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                   MESA_VK_DYNAMIC_RS_LINE_MODE) ||
       (cmd->state.dirty & TU_CMD_DIRTY_TES)) {
      tu6_update_msaa_disable(cmd);
   }

   if (BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                   MESA_VK_DYNAMIC_MS_RASTERIZATION_SAMPLES)) {
      tu6_update_msaa(cmd);
   }

   bool dirty_fs_params = false;
   if (BITSET_TEST(cmd->vk.dynamic_graphics_state.dirty,
                   MESA_VK_DYNAMIC_MS_RASTERIZATION_SAMPLES) ||
       (cmd->state.dirty & (TU_CMD_DIRTY_PROGRAM | TU_CMD_DIRTY_FDM))) {
      tu6_emit_fs_params(cmd);
      dirty_fs_params = true;
   }

   /* for the first draw in a renderpass, re-emit all the draw states
    *
    * and if a draw-state disabling path (CmdClearAttachments 3D fallback) was
    * used, then draw states must be re-emitted. note however this only happens
    * in the sysmem path, so this can be skipped this for the gmem path (TODO)
    *
    * the two input attachment states are excluded because secondary command
    * buffer doesn't have a state ib to restore it, and not re-emitting them
    * is OK since CmdClearAttachments won't disable/overwrite them
    */
   if (dirty & TU_CMD_DIRTY_DRAW_STATE) {
      tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 3 * (TU_DRAW_STATE_COUNT - 2));

      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_PROGRAM_CONFIG, program->config_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS, program->vs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS_BINNING, program->vs_binning_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_HS, program->hs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DS, program->ds_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_GS, program->gs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_GS_BINNING, program->gs_binning_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_FS, program->fs_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VPC, program->vpc_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_PRIM_MODE_SYSMEM, cmd->state.prim_order_sysmem);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_PRIM_MODE_GMEM, cmd->state.prim_order_gmem);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_CONST, cmd->state.shader_const);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DESC_SETS, cmd->state.desc_sets);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DESC_SETS_LOAD, cmd->state.load_state);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VB, cmd->state.vertex_buffers);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS_PARAMS, cmd->state.vs_params);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_FS_PARAMS, cmd->state.fs_params);
      tu_cs_emit_draw_state(cs, TU_DRAW_STATE_LRZ_AND_DEPTH_PLANE, cmd->state.lrz_and_depth_plane_state);

      for (uint32_t i = 0; i < ARRAY_SIZE(cmd->state.dynamic_state); i++) {
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DYNAMIC + i,
                               cmd->state.dynamic_state[i]);
      }
   } else {
      /* emit draw states that were just updated */
      uint32_t draw_state_count =
         util_bitcount(dynamic_draw_state_dirty) +
         ((dirty & TU_CMD_DIRTY_SHADER_CONSTS) ? 1 : 0) +
         ((dirty & TU_CMD_DIRTY_DESC_SETS) ? 1 : 0) +
         ((dirty & TU_CMD_DIRTY_VERTEX_BUFFERS) ? 1 : 0) +
         ((dirty & TU_CMD_DIRTY_VS_PARAMS) ? 1 : 0) +
         (dirty_fs_params ? 1 : 0) +
         (dirty_lrz ? 1 : 0);

      if (draw_state_count > 0)
         tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 3 * draw_state_count);

      if (dirty & TU_CMD_DIRTY_SHADER_CONSTS)
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_CONST, cmd->state.shader_const);
      if (dirty & TU_CMD_DIRTY_DESC_SETS) {
         /* tu6_emit_descriptor_sets emitted the cmd->state.desc_sets draw state. */
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DESC_SETS_LOAD, cmd->state.load_state);
      }
      if (dirty & TU_CMD_DIRTY_VERTEX_BUFFERS)
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VB, cmd->state.vertex_buffers);
      u_foreach_bit (i, dynamic_draw_state_dirty) {
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_DYNAMIC + i,
                               cmd->state.dynamic_state[i]);
      }
      if (dirty & TU_CMD_DIRTY_VS_PARAMS)
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS_PARAMS, cmd->state.vs_params);
      if (dirty_fs_params)
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_FS_PARAMS, cmd->state.fs_params);
      if (dirty_lrz) {
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_LRZ_AND_DEPTH_PLANE, cmd->state.lrz_and_depth_plane_state);
      }
   }

   tu_cs_sanity_check(cs);

   /* There are too many graphics dirty bits to list here, so just list the
    * bits to preserve instead. The only things not emitted here are
    * compute-related state.
    */
   cmd->state.dirty &= TU_CMD_DIRTY_COMPUTE_DESC_SETS;
   BITSET_ZERO(cmd->vk.dynamic_graphics_state.dirty);
   return VK_SUCCESS;
}

static uint32_t
tu_draw_initiator(struct tu_cmd_buffer *cmd, enum pc_di_src_sel src_sel)
{
   enum pc_di_primtype primtype =
      tu6_primtype((VkPrimitiveTopology)cmd->vk.dynamic_graphics_state.ia.primitive_topology);

   if (primtype == DI_PT_PATCHES0)
      primtype = (enum pc_di_primtype) (primtype +
                                        cmd->vk.dynamic_graphics_state.ts.patch_control_points);

   uint32_t initiator =
      CP_DRAW_INDX_OFFSET_0_PRIM_TYPE(primtype) |
      CP_DRAW_INDX_OFFSET_0_SOURCE_SELECT(src_sel) |
      CP_DRAW_INDX_OFFSET_0_INDEX_SIZE((enum a4xx_index_size) cmd->state.index_size) |
      CP_DRAW_INDX_OFFSET_0_VIS_CULL(USE_VISIBILITY);

   if (cmd->state.shaders[MESA_SHADER_GEOMETRY]->variant)
      initiator |= CP_DRAW_INDX_OFFSET_0_GS_ENABLE;

   const struct tu_shader *tes = cmd->state.shaders[MESA_SHADER_TESS_EVAL];
   if (tes->variant) {
      switch (tes->variant->key.tessellation) {
      case IR3_TESS_TRIANGLES:
         initiator |= CP_DRAW_INDX_OFFSET_0_PATCH_TYPE(TESS_TRIANGLES) |
                      CP_DRAW_INDX_OFFSET_0_TESS_ENABLE;
         break;
      case IR3_TESS_ISOLINES:
         initiator |= CP_DRAW_INDX_OFFSET_0_PATCH_TYPE(TESS_ISOLINES) |
                      CP_DRAW_INDX_OFFSET_0_TESS_ENABLE;
         break;
      case IR3_TESS_QUADS:
         initiator |= CP_DRAW_INDX_OFFSET_0_PATCH_TYPE(TESS_QUADS) |
                      CP_DRAW_INDX_OFFSET_0_TESS_ENABLE;
         break;
      }
   }
   return initiator;
}


static uint32_t
vs_params_offset(struct tu_cmd_buffer *cmd)
{
   const struct tu_program_descriptor_linkage *link =
      &cmd->state.program.link[MESA_SHADER_VERTEX];
   const struct ir3_const_state *const_state = &link->const_state;

   if (const_state->offsets.driver_param >= link->constlen)
      return 0;

   /* this layout is required by CP_DRAW_INDIRECT_MULTI */
   STATIC_ASSERT(IR3_DP_DRAWID == 0);
   STATIC_ASSERT(IR3_DP_VTXID_BASE == 1);
   STATIC_ASSERT(IR3_DP_INSTID_BASE == 2);

   /* 0 means disabled for CP_DRAW_INDIRECT_MULTI */
   assert(const_state->offsets.driver_param != 0);

   return const_state->offsets.driver_param;
}

static void
tu6_emit_empty_vs_params(struct tu_cmd_buffer *cmd)
{
   if (cmd->state.vs_params.iova) {
      cmd->state.vs_params = (struct tu_draw_state) {};
      cmd->state.dirty |= TU_CMD_DIRTY_VS_PARAMS;
   }
}

static void
tu6_emit_vs_params(struct tu_cmd_buffer *cmd,
                   uint32_t draw_id,
                   uint32_t vertex_offset,
                   uint32_t first_instance)
{
   uint32_t offset = vs_params_offset(cmd);

   /* Beside re-emitting params when they are changed, we should re-emit
    * them after constants are invalidated via HLSQ_INVALIDATE_CMD or after we
    * emit an empty vs params.
    */
   if (!(cmd->state.dirty & (TU_CMD_DIRTY_DRAW_STATE | TU_CMD_DIRTY_VS_PARAMS |
                             TU_CMD_DIRTY_PROGRAM)) &&
       cmd->state.vs_params.iova &&
       (offset == 0 || draw_id == cmd->state.last_vs_params.draw_id) &&
       vertex_offset == cmd->state.last_vs_params.vertex_offset &&
       first_instance == cmd->state.last_vs_params.first_instance) {
      return;
   }

   struct tu_cs cs;
   VkResult result = tu_cs_begin_sub_stream(&cmd->sub_cs, 3 + (offset ? 8 : 0), &cs);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   tu_cs_emit_regs(&cs,
                   A6XX_VFD_INDEX_OFFSET(vertex_offset),
                   A6XX_VFD_INSTANCE_START_OFFSET(first_instance));

   if (offset) {
      tu_cs_emit_pkt7(&cs, CP_LOAD_STATE6_GEOM, 3 + 4);
      tu_cs_emit(&cs, CP_LOAD_STATE6_0_DST_OFF(offset) |
            CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
            CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
            CP_LOAD_STATE6_0_STATE_BLOCK(SB6_VS_SHADER) |
            CP_LOAD_STATE6_0_NUM_UNIT(1));
      tu_cs_emit(&cs, 0);
      tu_cs_emit(&cs, 0);

      tu_cs_emit(&cs, draw_id);
      tu_cs_emit(&cs, vertex_offset);
      tu_cs_emit(&cs, first_instance);
      tu_cs_emit(&cs, 0);
   }

   cmd->state.last_vs_params.vertex_offset = vertex_offset;
   cmd->state.last_vs_params.first_instance = first_instance;
   cmd->state.last_vs_params.draw_id = draw_id;

   struct tu_cs_entry entry = tu_cs_end_sub_stream(&cmd->sub_cs, &cs);
   cmd->state.vs_params = (struct tu_draw_state) {entry.bo->iova + entry.offset, entry.size / 4};

   cmd->state.dirty |= TU_CMD_DIRTY_VS_PARAMS;
}

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDraw(VkCommandBuffer commandBuffer,
           uint32_t vertexCount,
           uint32_t instanceCount,
           uint32_t firstVertex,
           uint32_t firstInstance)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu6_emit_vs_params(cmd, 0, firstVertex, firstInstance);

   tu6_draw_common<CHIP>(cmd, cs, false, vertexCount);

   tu_cs_emit_pkt7(cs, CP_DRAW_INDX_OFFSET, 3);
   tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_AUTO_INDEX));
   tu_cs_emit(cs, instanceCount);
   tu_cs_emit(cs, vertexCount);
}
TU_GENX(tu_CmdDraw);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawMultiEXT(VkCommandBuffer commandBuffer,
                   uint32_t drawCount,
                   const VkMultiDrawInfoEXT *pVertexInfo,
                   uint32_t instanceCount,
                   uint32_t firstInstance,
                   uint32_t stride)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   if (!drawCount)
      return;

   bool has_tess = cmd->state.shaders[MESA_SHADER_TESS_CTRL]->variant;

   uint32_t max_vertex_count = 0;
   if (has_tess) {
      uint32_t i = 0;
      vk_foreach_multi_draw(draw, i, pVertexInfo, drawCount, stride) {
         max_vertex_count = MAX2(max_vertex_count, draw->vertexCount);
      }
   }

   uint32_t i = 0;
   vk_foreach_multi_draw(draw, i, pVertexInfo, drawCount, stride) {
      tu6_emit_vs_params(cmd, i, draw->firstVertex, firstInstance);

      if (i == 0)
         tu6_draw_common<CHIP>(cmd, cs, false, max_vertex_count);

      if (cmd->state.dirty & TU_CMD_DIRTY_VS_PARAMS) {
         tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 3);
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS_PARAMS, cmd->state.vs_params);
         cmd->state.dirty &= ~TU_CMD_DIRTY_VS_PARAMS;
      }

      tu_cs_emit_pkt7(cs, CP_DRAW_INDX_OFFSET, 3);
      tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_AUTO_INDEX));
      tu_cs_emit(cs, instanceCount);
      tu_cs_emit(cs, draw->vertexCount);
   }
}
TU_GENX(tu_CmdDrawMultiEXT);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawIndexed(VkCommandBuffer commandBuffer,
                  uint32_t indexCount,
                  uint32_t instanceCount,
                  uint32_t firstIndex,
                  int32_t vertexOffset,
                  uint32_t firstInstance)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu6_emit_vs_params(cmd, 0, vertexOffset, firstInstance);

   tu6_draw_common<CHIP>(cmd, cs, true, indexCount);

   tu_cs_emit_pkt7(cs, CP_DRAW_INDX_OFFSET, 7);
   tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_DMA));
   tu_cs_emit(cs, instanceCount);
   tu_cs_emit(cs, indexCount);
   tu_cs_emit(cs, firstIndex);
   tu_cs_emit_qw(cs, cmd->state.index_va);
   tu_cs_emit(cs, cmd->state.max_index_count);
}
TU_GENX(tu_CmdDrawIndexed);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer,
                          uint32_t drawCount,
                          const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                          uint32_t instanceCount,
                          uint32_t firstInstance,
                          uint32_t stride,
                          const int32_t *pVertexOffset)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   if (!drawCount)
      return;

   bool has_tess = cmd->state.shaders[MESA_SHADER_TESS_CTRL]->variant;

   uint32_t max_index_count = 0;
   if (has_tess) {
      uint32_t i = 0;
      vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
         max_index_count = MAX2(max_index_count, draw->indexCount);
      }
   }

   uint32_t i = 0;
   vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
      int32_t vertexOffset = pVertexOffset ? *pVertexOffset : draw->vertexOffset;
      tu6_emit_vs_params(cmd, i, vertexOffset, firstInstance);

      if (i == 0)
         tu6_draw_common<CHIP>(cmd, cs, true, max_index_count);

      if (cmd->state.dirty & TU_CMD_DIRTY_VS_PARAMS) {
         tu_cs_emit_pkt7(cs, CP_SET_DRAW_STATE, 3);
         tu_cs_emit_draw_state(cs, TU_DRAW_STATE_VS_PARAMS, cmd->state.vs_params);
         cmd->state.dirty &= ~TU_CMD_DIRTY_VS_PARAMS;
      }

      tu_cs_emit_pkt7(cs, CP_DRAW_INDX_OFFSET, 7);
      tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_DMA));
      tu_cs_emit(cs, instanceCount);
      tu_cs_emit(cs, draw->indexCount);
      tu_cs_emit(cs, draw->firstIndex);
      tu_cs_emit_qw(cs, cmd->state.index_va);
      tu_cs_emit(cs, cmd->state.max_index_count);
   }
}
TU_GENX(tu_CmdDrawMultiIndexedEXT);

/* Various firmware bugs/inconsistencies mean that some indirect draw opcodes
 * do not wait for WFI's to complete before executing. Add a WAIT_FOR_ME if
 * pending for these opcodes. This may result in a few extra WAIT_FOR_ME's
 * with these opcodes, but the alternative would add unnecessary WAIT_FOR_ME's
 * before draw opcodes that don't need it.
 */
static void
draw_wfm(struct tu_cmd_buffer *cmd)
{
   cmd->state.renderpass_cache.flush_bits |=
      cmd->state.renderpass_cache.pending_flush_bits & TU_CMD_FLAG_WAIT_FOR_ME;
   cmd->state.renderpass_cache.pending_flush_bits &= ~TU_CMD_FLAG_WAIT_FOR_ME;
}

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawIndirect(VkCommandBuffer commandBuffer,
                   VkBuffer _buffer,
                   VkDeviceSize offset,
                   uint32_t drawCount,
                   uint32_t stride)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buf, _buffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu6_emit_empty_vs_params(cmd);

   if (cmd->device->physical_device->info->a6xx.indirect_draw_wfm_quirk)
      draw_wfm(cmd);

   tu6_draw_common<CHIP>(cmd, cs, false, 0);

   tu_cs_emit_pkt7(cs, CP_DRAW_INDIRECT_MULTI, 6);
   tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_AUTO_INDEX));
   tu_cs_emit(cs, A6XX_CP_DRAW_INDIRECT_MULTI_1_OPCODE(INDIRECT_OP_NORMAL) |
                  A6XX_CP_DRAW_INDIRECT_MULTI_1_DST_OFF(vs_params_offset(cmd)));
   tu_cs_emit(cs, drawCount);
   tu_cs_emit_qw(cs, buf->iova + offset);
   tu_cs_emit(cs, stride);
}
TU_GENX(tu_CmdDrawIndirect);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer,
                          VkBuffer _buffer,
                          VkDeviceSize offset,
                          uint32_t drawCount,
                          uint32_t stride)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buf, _buffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu6_emit_empty_vs_params(cmd);

   if (cmd->device->physical_device->info->a6xx.indirect_draw_wfm_quirk)
      draw_wfm(cmd);

   tu6_draw_common<CHIP>(cmd, cs, true, 0);

   tu_cs_emit_pkt7(cs, CP_DRAW_INDIRECT_MULTI, 9);
   tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_DMA));
   tu_cs_emit(cs, A6XX_CP_DRAW_INDIRECT_MULTI_1_OPCODE(INDIRECT_OP_INDEXED) |
                  A6XX_CP_DRAW_INDIRECT_MULTI_1_DST_OFF(vs_params_offset(cmd)));
   tu_cs_emit(cs, drawCount);
   tu_cs_emit_qw(cs, cmd->state.index_va);
   tu_cs_emit(cs, cmd->state.max_index_count);
   tu_cs_emit_qw(cs, buf->iova + offset);
   tu_cs_emit(cs, stride);
}
TU_GENX(tu_CmdDrawIndexedIndirect);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawIndirectCount(VkCommandBuffer commandBuffer,
                        VkBuffer _buffer,
                        VkDeviceSize offset,
                        VkBuffer countBuffer,
                        VkDeviceSize countBufferOffset,
                        uint32_t drawCount,
                        uint32_t stride)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buf, _buffer);
   TU_FROM_HANDLE(tu_buffer, count_buf, countBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu6_emit_empty_vs_params(cmd);

   /* It turns out that the firmware we have for a650 only partially fixed the
    * problem with CP_DRAW_INDIRECT_MULTI not waiting for WFI's to complete
    * before reading indirect parameters. It waits for WFI's before reading
    * the draw parameters, but after reading the indirect count :(.
    */
   draw_wfm(cmd);

   tu6_draw_common<CHIP>(cmd, cs, false, 0);

   tu_cs_emit_pkt7(cs, CP_DRAW_INDIRECT_MULTI, 8);
   tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_AUTO_INDEX));
   tu_cs_emit(cs, A6XX_CP_DRAW_INDIRECT_MULTI_1_OPCODE(INDIRECT_OP_INDIRECT_COUNT) |
                  A6XX_CP_DRAW_INDIRECT_MULTI_1_DST_OFF(vs_params_offset(cmd)));
   tu_cs_emit(cs, drawCount);
   tu_cs_emit_qw(cs, buf->iova + offset);
   tu_cs_emit_qw(cs, count_buf->iova + countBufferOffset);
   tu_cs_emit(cs, stride);
}
TU_GENX(tu_CmdDrawIndirectCount);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer,
                               VkBuffer _buffer,
                               VkDeviceSize offset,
                               VkBuffer countBuffer,
                               VkDeviceSize countBufferOffset,
                               uint32_t drawCount,
                               uint32_t stride)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buf, _buffer);
   TU_FROM_HANDLE(tu_buffer, count_buf, countBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   tu6_emit_empty_vs_params(cmd);

   draw_wfm(cmd);

   tu6_draw_common<CHIP>(cmd, cs, true, 0);

   tu_cs_emit_pkt7(cs, CP_DRAW_INDIRECT_MULTI, 11);
   tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_DMA));
   tu_cs_emit(cs, A6XX_CP_DRAW_INDIRECT_MULTI_1_OPCODE(INDIRECT_OP_INDIRECT_COUNT_INDEXED) |
                  A6XX_CP_DRAW_INDIRECT_MULTI_1_DST_OFF(vs_params_offset(cmd)));
   tu_cs_emit(cs, drawCount);
   tu_cs_emit_qw(cs, cmd->state.index_va);
   tu_cs_emit(cs, cmd->state.max_index_count);
   tu_cs_emit_qw(cs, buf->iova + offset);
   tu_cs_emit_qw(cs, count_buf->iova + countBufferOffset);
   tu_cs_emit(cs, stride);
}
TU_GENX(tu_CmdDrawIndexedIndirectCount);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer,
                               uint32_t instanceCount,
                               uint32_t firstInstance,
                               VkBuffer _counterBuffer,
                               VkDeviceSize counterBufferOffset,
                               uint32_t counterOffset,
                               uint32_t vertexStride)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buf, _counterBuffer);
   struct tu_cs *cs = &cmd->draw_cs;

   /* All known firmware versions do not wait for WFI's with CP_DRAW_AUTO.
    * Plus, for the common case where the counter buffer is written by
    * vkCmdEndTransformFeedback, we need to wait for the CP_WAIT_MEM_WRITES to
    * complete which means we need a WAIT_FOR_ME anyway.
    */
   draw_wfm(cmd);

   tu6_emit_vs_params(cmd, 0, 0, firstInstance);

   tu6_draw_common<CHIP>(cmd, cs, false, 0);

   tu_cs_emit_pkt7(cs, CP_DRAW_AUTO, 6);
   if (CHIP == A6XX) {
      tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_AUTO_XFB));
   } else {
      tu_cs_emit(cs, tu_draw_initiator(cmd, DI_SRC_SEL_AUTO_INDEX));
      /* On a7xx the counter value and offset are shifted right by 2, so
       * the vertexStride should also be in units of dwords.
       */
      vertexStride = vertexStride >> 2;
   }
   tu_cs_emit(cs, instanceCount);
   tu_cs_emit_qw(cs, buf->iova + counterBufferOffset);
   tu_cs_emit(cs, counterOffset);
   tu_cs_emit(cs, vertexStride);
}
TU_GENX(tu_CmdDrawIndirectByteCountEXT);

struct tu_dispatch_info
{
   /**
    * Determine the layout of the grid (in block units) to be used.
    */
   uint32_t blocks[3];

   /**
    * A starting offset for the grid. If unaligned is set, the offset
    * must still be aligned.
    */
   uint32_t offsets[3];
   /**
    * Whether it's an unaligned compute dispatch.
    */
   bool unaligned;

   /**
    * Indirect compute parameters resource.
    */
   struct tu_buffer *indirect;
   uint64_t indirect_offset;
};

template <chip CHIP>
static void
tu_emit_compute_driver_params(struct tu_cmd_buffer *cmd,
                              struct tu_cs *cs,
                              const struct tu_dispatch_info *info)
{
   gl_shader_stage type = MESA_SHADER_COMPUTE;
   const struct tu_shader *shader = cmd->state.shaders[MESA_SHADER_COMPUTE];
   const struct ir3_shader_variant *variant = shader->variant;
   const struct ir3_const_state *const_state = variant->const_state;
   uint32_t offset = const_state->offsets.driver_param;
   unsigned subgroup_size = variant->info.subgroup_size;
   unsigned subgroup_shift = util_logbase2(subgroup_size);

   if (variant->constlen <= offset)
      return;

   uint32_t num_consts = MIN2(const_state->num_driver_params,
                              (variant->constlen - offset) * 4);

   if (!info->indirect) {
      uint32_t driver_params[12] = {
         [IR3_DP_NUM_WORK_GROUPS_X] = info->blocks[0],
         [IR3_DP_NUM_WORK_GROUPS_Y] = info->blocks[1],
         [IR3_DP_NUM_WORK_GROUPS_Z] = info->blocks[2],
         [IR3_DP_WORK_DIM] = 0,
         [IR3_DP_BASE_GROUP_X] = info->offsets[0],
         [IR3_DP_BASE_GROUP_Y] = info->offsets[1],
         [IR3_DP_BASE_GROUP_Z] = info->offsets[2],
         [IR3_DP_CS_SUBGROUP_SIZE] = subgroup_size,
         [IR3_DP_LOCAL_GROUP_SIZE_X] = 0,
         [IR3_DP_LOCAL_GROUP_SIZE_Y] = 0,
         [IR3_DP_LOCAL_GROUP_SIZE_Z] = 0,
         [IR3_DP_SUBGROUP_ID_SHIFT] = subgroup_shift,
      };

      assert(num_consts <= ARRAY_SIZE(driver_params));

      /* push constants */
      tu_cs_emit_pkt7(cs, tu6_stage2opcode(type), 3 + num_consts);
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(offset) |
                 CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                 CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
                 CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(type)) |
                 CP_LOAD_STATE6_0_NUM_UNIT(num_consts / 4));
      tu_cs_emit(cs, 0);
      tu_cs_emit(cs, 0);
      uint32_t i;
      for (i = 0; i < num_consts; i++)
         tu_cs_emit(cs, driver_params[i]);
   } else if (!(info->indirect_offset & 0xf)) {
      tu_cs_emit_pkt7(cs, tu6_stage2opcode(type), 3);
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(offset) |
                  CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                  CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                  CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(type)) |
                  CP_LOAD_STATE6_0_NUM_UNIT(1));
      tu_cs_emit_qw(cs, info->indirect->iova + info->indirect_offset);
   } else {
      /* Vulkan guarantees only 4 byte alignment for indirect_offset.
       * However, CP_LOAD_STATE.EXT_SRC_ADDR needs 16 byte alignment.
       */

      uint64_t indirect_iova = info->indirect->iova + info->indirect_offset;

      for (uint32_t i = 0; i < 3; i++) {
         tu_cs_emit_pkt7(cs, CP_MEM_TO_MEM, 5);
         tu_cs_emit(cs, 0);
         tu_cs_emit_qw(cs, global_iova_arr(cmd, cs_indirect_xyz, i));
         tu_cs_emit_qw(cs, indirect_iova + i * 4);
      }

      tu_cs_emit_pkt7(cs, CP_WAIT_MEM_WRITES, 0);
      tu_emit_event_write<CHIP>(cmd, cs, FD_CACHE_INVALIDATE);

      tu_cs_emit_pkt7(cs, tu6_stage2opcode(type), 3);
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(offset) |
                  CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                  CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                  CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(type)) |
                  CP_LOAD_STATE6_0_NUM_UNIT(1));
      tu_cs_emit_qw(cs, global_iova(cmd, cs_indirect_xyz[0]));
   }

   /* Fill out IR3_DP_CS_SUBGROUP_SIZE and IR3_DP_SUBGROUP_ID_SHIFT for
    * indirect dispatch.
    */
   if (info->indirect && num_consts > IR3_DP_BASE_GROUP_X) {
      bool emit_local = num_consts > IR3_DP_LOCAL_GROUP_SIZE_X;
      tu_cs_emit_pkt7(cs, tu6_stage2opcode(type), 7 + (emit_local ? 4 : 0));
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(offset + (IR3_DP_BASE_GROUP_X / 4)) |
                 CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                 CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
                 CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(type)) |
                 CP_LOAD_STATE6_0_NUM_UNIT((num_consts - IR3_DP_BASE_GROUP_X) / 4));
      tu_cs_emit_qw(cs, 0);
      tu_cs_emit(cs, 0); /* BASE_GROUP_X */
      tu_cs_emit(cs, 0); /* BASE_GROUP_Y */
      tu_cs_emit(cs, 0); /* BASE_GROUP_Z */
      tu_cs_emit(cs, subgroup_size);
      if (emit_local) {
         assert(num_consts == align(IR3_DP_SUBGROUP_ID_SHIFT, 4));
         tu_cs_emit(cs, 0); /* LOCAL_GROUP_SIZE_X */
         tu_cs_emit(cs, 0); /* LOCAL_GROUP_SIZE_Y */
         tu_cs_emit(cs, 0); /* LOCAL_GROUP_SIZE_Z */
         tu_cs_emit(cs, subgroup_shift);
      }
   }
}

template <chip CHIP>
static void
tu_dispatch(struct tu_cmd_buffer *cmd,
            const struct tu_dispatch_info *info)
{
   if (!info->indirect &&
       (info->blocks[0] == 0 || info->blocks[1] == 0 || info->blocks[2] == 0))
      return;

   struct tu_cs *cs = &cmd->cs;
   struct tu_shader *shader = cmd->state.shaders[MESA_SHADER_COMPUTE];

   bool emit_instrlen_workaround =
      shader->variant->instrlen >
      cmd->device->physical_device->info->a6xx.instr_cache_size;

   /* We don't use draw states for dispatches, so the bound pipeline
    * could be overwritten by reg stomping in a renderpass or blit.
    */
   if (cmd->device->dbg_renderpass_stomp_cs) {
      tu_cs_emit_state_ib(&cmd->cs, shader->state);
   }

   /* There appears to be a HW bug where in some rare circumstances it appears
    * to accidentally use the FS instrlen instead of the CS instrlen, which
    * affects all known gens. Based on various experiments it appears that the
    * issue is that when prefetching a branch destination and there is a cache
    * miss, when fetching from memory the HW bounds-checks the fetch against
    * SP_CS_INSTRLEN, except when one of the two register contexts is active
    * it accidentally fetches SP_FS_INSTRLEN from the other (inactive)
    * context. To workaround it we set the FS instrlen here and do a dummy
    * event to roll the context (because it fetches SP_FS_INSTRLEN from the
    * "wrong" context). Because the bug seems to involve cache misses, we
    * don't emit this if the entire CS program fits in cache, which will
    * hopefully be the majority of cases.
    *
    * See https://gitlab.freedesktop.org/mesa/mesa/-/issues/5892
    */
   if (emit_instrlen_workaround) {
      tu_cs_emit_regs(cs, A6XX_SP_FS_INSTRLEN(shader->variant->instrlen));
      tu_emit_event_write<CHIP>(cmd, cs, FD_LABEL);
   }

   /* TODO: We could probably flush less if we add a compute_flush_bits
    * bitfield.
    */
   tu_emit_cache_flush<CHIP>(cmd);

   /* note: no reason to have this in a separate IB */
   tu_cs_emit_state_ib(cs, tu_emit_consts(cmd, true));

   tu_emit_compute_driver_params<CHIP>(cmd, cs, info);

   if (cmd->state.dirty & TU_CMD_DIRTY_COMPUTE_DESC_SETS) {
      tu6_emit_descriptor_sets<CHIP>(cmd, VK_PIPELINE_BIND_POINT_COMPUTE);
      tu_cs_emit_state_ib(cs, cmd->state.compute_load_state);
   }

   cmd->state.dirty &= ~TU_CMD_DIRTY_COMPUTE_DESC_SETS;

   tu_cs_emit_pkt7(cs, CP_SET_MARKER, 1);
   tu_cs_emit(cs, A6XX_CP_SET_MARKER_0_MODE(RM6_COMPUTE));

   const uint16_t *local_size = shader->variant->local_size;
   const uint32_t *num_groups = info->blocks;
   tu_cs_emit_regs(cs,
                   HLSQ_CS_NDRANGE_0(CHIP, .kerneldim = 3,
                                           .localsizex = local_size[0] - 1,
                                           .localsizey = local_size[1] - 1,
                                           .localsizez = local_size[2] - 1),
                   HLSQ_CS_NDRANGE_1(CHIP, .globalsize_x = local_size[0] * num_groups[0]),
                   HLSQ_CS_NDRANGE_2(CHIP, .globaloff_x = 0),
                   HLSQ_CS_NDRANGE_3(CHIP, .globalsize_y = local_size[1] * num_groups[1]),
                   HLSQ_CS_NDRANGE_4(CHIP, .globaloff_y = 0),
                   HLSQ_CS_NDRANGE_5(CHIP, .globalsize_z = local_size[2] * num_groups[2]),
                   HLSQ_CS_NDRANGE_6(CHIP, .globaloff_z = 0));

   tu_cs_emit_regs(cs,
                   HLSQ_CS_KERNEL_GROUP_X(CHIP, 1),
                   HLSQ_CS_KERNEL_GROUP_Y(CHIP, 1),
                   HLSQ_CS_KERNEL_GROUP_Z(CHIP, 1));

   trace_start_compute(&cmd->trace, cs, info->indirect != NULL, local_size[0],
                       local_size[1], local_size[2], info->blocks[0],
                       info->blocks[1], info->blocks[2]);

   if (info->indirect) {
      uint64_t iova = info->indirect->iova + info->indirect_offset;

      tu_cs_emit_pkt7(cs, CP_EXEC_CS_INDIRECT, 4);
      tu_cs_emit(cs, 0x00000000);
      tu_cs_emit_qw(cs, iova);
      tu_cs_emit(cs,
                 A5XX_CP_EXEC_CS_INDIRECT_3_LOCALSIZEX(local_size[0] - 1) |
                 A5XX_CP_EXEC_CS_INDIRECT_3_LOCALSIZEY(local_size[1] - 1) |
                 A5XX_CP_EXEC_CS_INDIRECT_3_LOCALSIZEZ(local_size[2] - 1));
   } else {
      tu_cs_emit_pkt7(cs, CP_EXEC_CS, 4);
      tu_cs_emit(cs, 0x00000000);
      tu_cs_emit(cs, CP_EXEC_CS_1_NGROUPS_X(info->blocks[0]));
      tu_cs_emit(cs, CP_EXEC_CS_2_NGROUPS_Y(info->blocks[1]));
      tu_cs_emit(cs, CP_EXEC_CS_3_NGROUPS_Z(info->blocks[2]));
   }

   trace_end_compute(&cmd->trace, cs);

   /* For the workaround above, because it's using the "wrong" context for
    * SP_FS_INSTRLEN we should emit another dummy event write to avoid a
    * potential race between writing the register and the CP_EXEC_CS we just
    * did. We don't need to reset the register because it will be re-emitted
    * anyway when the next renderpass starts.
    */
   if (emit_instrlen_workaround) {
      tu_emit_event_write<CHIP>(cmd, cs, FD_LABEL);
   }

   tu_cs_emit_wfi(cs);
}

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDispatchBase(VkCommandBuffer commandBuffer,
                   uint32_t base_x,
                   uint32_t base_y,
                   uint32_t base_z,
                   uint32_t x,
                   uint32_t y,
                   uint32_t z)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, commandBuffer);
   struct tu_dispatch_info info = {};

   info.blocks[0] = x;
   info.blocks[1] = y;
   info.blocks[2] = z;

   info.offsets[0] = base_x;
   info.offsets[1] = base_y;
   info.offsets[2] = base_z;
   tu_dispatch<CHIP>(cmd_buffer, &info);
}
TU_GENX(tu_CmdDispatchBase);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdDispatchIndirect(VkCommandBuffer commandBuffer,
                       VkBuffer _buffer,
                       VkDeviceSize offset)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buffer, _buffer);
   struct tu_dispatch_info info = {};

   info.indirect = buffer;
   info.indirect_offset = offset;

   tu_dispatch<CHIP>(cmd_buffer, &info);
}
TU_GENX(tu_CmdDispatchIndirect);

VKAPI_ATTR void VKAPI_CALL
tu_CmdEndRenderPass2(VkCommandBuffer commandBuffer,
                     const VkSubpassEndInfo *pSubpassEndInfo)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, commandBuffer);

   if (TU_DEBUG(DYNAMIC)) {
      vk_common_CmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
      return;
   }

   tu_cs_end(&cmd_buffer->draw_cs);
   tu_cs_end(&cmd_buffer->draw_epilogue_cs);
   TU_CALLX(cmd_buffer->device, tu_cmd_render)(cmd_buffer);

   cmd_buffer->state.cache.pending_flush_bits |=
      cmd_buffer->state.renderpass_cache.pending_flush_bits;
   tu_subpass_barrier(cmd_buffer, &cmd_buffer->state.pass->end_barrier, true);

   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer->state.attachments);

   tu_reset_render_pass(cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdEndRendering(VkCommandBuffer commandBuffer)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, commandBuffer);

   if (cmd_buffer->state.suspending)
      cmd_buffer->state.suspended_pass.lrz = cmd_buffer->state.lrz;

   if (!cmd_buffer->state.suspending) {
      tu_cs_end(&cmd_buffer->draw_cs);
      tu_cs_end(&cmd_buffer->draw_epilogue_cs);

      if (cmd_buffer->state.suspend_resume == SR_IN_PRE_CHAIN) {
         cmd_buffer->trace_renderpass_end = u_trace_end_iterator(&cmd_buffer->trace);
         tu_save_pre_chain(cmd_buffer);

         /* Even we don't call tu_cmd_render here, renderpass is finished
          * and draw states should be disabled.
          */
         tu_disable_draw_states(cmd_buffer, &cmd_buffer->cs);
      } else {
         TU_CALLX(cmd_buffer->device, tu_cmd_render)(cmd_buffer);
      }

      tu_reset_render_pass(cmd_buffer);
   }

   if (cmd_buffer->state.resuming && !cmd_buffer->state.suspending) {
      /* exiting suspend/resume chain */
      switch (cmd_buffer->state.suspend_resume) {
      case SR_IN_CHAIN:
         cmd_buffer->state.suspend_resume = SR_NONE;
         break;
      case SR_IN_PRE_CHAIN:
      case SR_IN_CHAIN_AFTER_PRE_CHAIN:
         cmd_buffer->state.suspend_resume = SR_AFTER_PRE_CHAIN;
         break;
      default:
         unreachable("suspending render pass not followed by resuming pass");
      }
   }
}

static void
tu_barrier(struct tu_cmd_buffer *cmd,
           const VkDependencyInfo *dep_info)
{
   VkPipelineStageFlags2 srcStage = 0;
   VkPipelineStageFlags2 dstStage = 0;
   BITMASK_ENUM(tu_cmd_access_mask) src_flags = 0;
   BITMASK_ENUM(tu_cmd_access_mask) dst_flags = 0;

   /* Inside a renderpass, we don't know yet whether we'll be using sysmem
    * so we have to use the sysmem flushes.
    */
   bool gmem = cmd->state.ccu_state == TU_CMD_CCU_GMEM &&
      !cmd->state.pass;


   for (uint32_t i = 0; i < dep_info->memoryBarrierCount; i++) {
      VkPipelineStageFlags2 sanitized_src_stage =
         sanitize_src_stage(dep_info->pMemoryBarriers[i].srcStageMask);
      VkPipelineStageFlags2 sanitized_dst_stage =
         sanitize_dst_stage(dep_info->pMemoryBarriers[i].dstStageMask);
      src_flags |= vk2tu_access(dep_info->pMemoryBarriers[i].srcAccessMask,
                                sanitized_src_stage, false, gmem);
      dst_flags |= vk2tu_access(dep_info->pMemoryBarriers[i].dstAccessMask,
                                sanitized_dst_stage, false, gmem);
      srcStage |= sanitized_src_stage;
      dstStage |= sanitized_dst_stage;
   }

   for (uint32_t i = 0; i < dep_info->bufferMemoryBarrierCount; i++) {
      VkPipelineStageFlags2 sanitized_src_stage =
         sanitize_src_stage(dep_info->pBufferMemoryBarriers[i].srcStageMask);
      VkPipelineStageFlags2 sanitized_dst_stage =
         sanitize_dst_stage(dep_info->pBufferMemoryBarriers[i].dstStageMask);
      src_flags |= vk2tu_access(dep_info->pBufferMemoryBarriers[i].srcAccessMask,
                                sanitized_src_stage, false, gmem);
      dst_flags |= vk2tu_access(dep_info->pBufferMemoryBarriers[i].dstAccessMask,
                                sanitized_dst_stage, false, gmem);
      srcStage |= sanitized_src_stage;
      dstStage |= sanitized_dst_stage;
   }

   for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; i++) {
      VkImageLayout old_layout = dep_info->pImageMemoryBarriers[i].oldLayout;
      if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
         /* The underlying memory for this image may have been used earlier
          * within the same queue submission for a different image, which
          * means that there may be old, stale cache entries which are in the
          * "wrong" location, which could cause problems later after writing
          * to the image. We don't want these entries being flushed later and
          * overwriting the actual image, so we need to flush the CCU.
          */
         TU_FROM_HANDLE(tu_image, image, dep_info->pImageMemoryBarriers[i].image);

         if (vk_format_is_depth_or_stencil(image->vk.format)) {
            src_flags |= TU_ACCESS_CCU_DEPTH_INCOHERENT_WRITE;
         } else {
            src_flags |= TU_ACCESS_CCU_COLOR_INCOHERENT_WRITE;
         }
      }
      VkPipelineStageFlags2 sanitized_src_stage =
         sanitize_src_stage(dep_info->pImageMemoryBarriers[i].srcStageMask);
      VkPipelineStageFlags2 sanitized_dst_stage =
         sanitize_dst_stage(dep_info->pImageMemoryBarriers[i].dstStageMask);
      src_flags |= vk2tu_access(dep_info->pImageMemoryBarriers[i].srcAccessMask,
                                sanitized_src_stage, true, gmem);
      dst_flags |= vk2tu_access(dep_info->pImageMemoryBarriers[i].dstAccessMask,
                                sanitized_dst_stage, true, gmem);
      srcStage |= sanitized_src_stage;
      dstStage |= sanitized_dst_stage;
   }

   if (cmd->state.pass) {
      const VkPipelineStageFlags framebuffer_space_stages =
         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
         VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      /* We cannot have non-by-region "fb-space to fb-space" barriers.
       *
       * From the Vulkan 1.2.185 spec, section 7.6.1 "Subpass Self-dependency":
       *
       *    If the source and destination stage masks both include
       *    framebuffer-space stages, then dependencyFlags must include
       *    VK_DEPENDENCY_BY_REGION_BIT.
       *    [...]
       *    Each of the synchronization scopes and access scopes of a
       *    vkCmdPipelineBarrier2 or vkCmdPipelineBarrier command inside
       *    a render pass instance must be a subset of the scopes of one of
       *    the self-dependencies for the current subpass.
       *
       *    If the self-dependency has VK_DEPENDENCY_BY_REGION_BIT or
       *    VK_DEPENDENCY_VIEW_LOCAL_BIT set, then so must the pipeline barrier.
       *
       * By-region barriers are ok for gmem. All other barriers would involve
       * vtx stages which are NOT ok for gmem rendering.
       * See dep_invalid_for_gmem().
       */
      if ((srcStage & ~framebuffer_space_stages) ||
          (dstStage & ~framebuffer_space_stages)) {
         cmd->state.rp.disable_gmem = true;
      }
   }

   struct tu_cache_state *cache =
      cmd->state.pass  ? &cmd->state.renderpass_cache : &cmd->state.cache;
   tu_flush_for_access(cache, src_flags, dst_flags);

   enum tu_stage src_stage = vk2tu_src_stage(srcStage);
   enum tu_stage dst_stage = vk2tu_dst_stage(dstStage);
   tu_flush_for_stage(cache, src_stage, dst_stage);
}

VKAPI_ATTR void VKAPI_CALL
tu_CmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                       const VkDependencyInfo *pDependencyInfo)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, commandBuffer);

   tu_barrier(cmd_buffer, pDependencyInfo);
}

template <chip CHIP>
static void
write_event(struct tu_cmd_buffer *cmd, struct tu_event *event,
            VkPipelineStageFlags2 stageMask, unsigned value)
{
   struct tu_cs *cs = &cmd->cs;

   /* vkCmdSetEvent/vkCmdResetEvent cannot be called inside a render pass */
   assert(!cmd->state.pass);

   tu_emit_cache_flush<CHIP>(cmd);

   /* Flags that only require a top-of-pipe event. DrawIndirect parameters are
    * read by the CP, so the draw indirect stage counts as top-of-pipe too.
    */
   VkPipelineStageFlags2 top_of_pipe_flags =
      VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT |
      VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;

   if (!(stageMask & ~top_of_pipe_flags)) {
      tu_cs_emit_pkt7(cs, CP_MEM_WRITE, 3);
      tu_cs_emit_qw(cs, event->bo->iova); /* ADDR_LO/HI */
      tu_cs_emit(cs, value);
   } else {
      /* Use a RB_DONE_TS event to wait for everything to complete. */
      if (CHIP == A6XX) {
         tu_cs_emit_pkt7(cs, CP_EVENT_WRITE, 4);
         tu_cs_emit(cs, CP_EVENT_WRITE_0_EVENT(RB_DONE_TS));
      } else {
         tu_cs_emit_pkt7(cs, CP_EVENT_WRITE7, 4);
         tu_cs_emit(cs, CP_EVENT_WRITE7_0(.event = RB_DONE_TS,
                                          .write_src = EV_WRITE_USER_32B,
                                          .write_dst = EV_DST_RAM,
                                          .write_enabled = true).value);
      }

      tu_cs_emit_qw(cs, event->bo->iova);
      tu_cs_emit(cs, value);
   }
}

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdSetEvent2(VkCommandBuffer commandBuffer,
                VkEvent _event,
                const VkDependencyInfo *pDependencyInfo)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_event, event, _event);
   VkPipelineStageFlags2 src_stage_mask = 0;

   for (uint32_t i = 0; i < pDependencyInfo->memoryBarrierCount; i++)
      src_stage_mask |= pDependencyInfo->pMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->bufferMemoryBarrierCount; i++)
      src_stage_mask |= pDependencyInfo->pBufferMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; i++)
      src_stage_mask |= pDependencyInfo->pImageMemoryBarriers[i].srcStageMask;

   write_event<CHIP>(cmd, event, src_stage_mask, 1);
}
TU_GENX(tu_CmdSetEvent2);

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdResetEvent2(VkCommandBuffer commandBuffer,
                  VkEvent _event,
                  VkPipelineStageFlags2 stageMask)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_event, event, _event);

   write_event<CHIP>(cmd, event, stageMask, 0);
}
TU_GENX(tu_CmdResetEvent2);

VKAPI_ATTR void VKAPI_CALL
tu_CmdWaitEvents2(VkCommandBuffer commandBuffer,
                  uint32_t eventCount,
                  const VkEvent *pEvents,
                  const VkDependencyInfo* pDependencyInfos)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   struct tu_cs *cs = cmd->state.pass ? &cmd->draw_cs : &cmd->cs;

   for (uint32_t i = 0; i < eventCount; i++) {
      TU_FROM_HANDLE(tu_event, event, pEvents[i]);

      tu_cs_emit_pkt7(cs, CP_WAIT_REG_MEM, 6);
      tu_cs_emit(cs, CP_WAIT_REG_MEM_0_FUNCTION(WRITE_EQ) |
                     CP_WAIT_REG_MEM_0_POLL(POLL_MEMORY));
      tu_cs_emit_qw(cs, event->bo->iova); /* POLL_ADDR_LO/HI */
      tu_cs_emit(cs, CP_WAIT_REG_MEM_3_REF(1));
      tu_cs_emit(cs, CP_WAIT_REG_MEM_4_MASK(~0u));
      tu_cs_emit(cs, CP_WAIT_REG_MEM_5_DELAY_LOOP_CYCLES(20));
   }

   tu_barrier(cmd, pDependencyInfos);
}

template <chip CHIP>
VKAPI_ATTR void VKAPI_CALL
tu_CmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                   const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);

   cmd->state.predication_active = true;

   struct tu_cs *cs = cmd->state.pass ? &cmd->draw_cs : &cmd->cs;

   tu_cs_emit_pkt7(cs, CP_DRAW_PRED_ENABLE_GLOBAL, 1);
   tu_cs_emit(cs, 1);

   /* Wait for any writes to the predicate to land */
   if (cmd->state.pass)
      tu_emit_cache_flush_renderpass<CHIP>(cmd);
   else
      tu_emit_cache_flush<CHIP>(cmd);

   TU_FROM_HANDLE(tu_buffer, buf, pConditionalRenderingBegin->buffer);
   uint64_t iova = buf->iova + pConditionalRenderingBegin->offset;

   /* qcom doesn't support 32-bit reference values, only 64-bit, but Vulkan
    * mandates 32-bit comparisons. Our workaround is to copy the the reference
    * value to the low 32-bits of a location where the high 32 bits are known
    * to be 0 and then compare that.
    */
   tu_cs_emit_pkt7(cs, CP_MEM_TO_MEM, 5);
   tu_cs_emit(cs, 0);
   tu_cs_emit_qw(cs, global_iova(cmd, predicate));
   tu_cs_emit_qw(cs, iova);

   tu_cs_emit_pkt7(cs, CP_WAIT_MEM_WRITES, 0);
   tu_cs_emit_pkt7(cs, CP_WAIT_FOR_ME, 0);

   bool inv = pConditionalRenderingBegin->flags & VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT;
   tu_cs_emit_pkt7(cs, CP_DRAW_PRED_SET, 3);
   tu_cs_emit(cs, CP_DRAW_PRED_SET_0_SRC(PRED_SRC_MEM) |
                  CP_DRAW_PRED_SET_0_TEST(inv ? EQ_0_PASS : NE_0_PASS));
   tu_cs_emit_qw(cs, global_iova(cmd, predicate));
}
TU_GENX(tu_CmdBeginConditionalRenderingEXT);

VKAPI_ATTR void VKAPI_CALL
tu_CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer)
{
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);

   cmd->state.predication_active = false;

   struct tu_cs *cs = cmd->state.pass ? &cmd->draw_cs : &cmd->cs;

   tu_cs_emit_pkt7(cs, CP_DRAW_PRED_ENABLE_GLOBAL, 1);
   tu_cs_emit(cs, 0);
}

template <chip CHIP>
void
tu_CmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer,
                            VkPipelineStageFlagBits2 pipelineStage,
                            VkBuffer dstBuffer,
                            VkDeviceSize dstOffset,
                            uint32_t marker)
{
   /* Almost the same as write_event, but also allowed in renderpass */
   TU_FROM_HANDLE(tu_cmd_buffer, cmd, commandBuffer);
   TU_FROM_HANDLE(tu_buffer, buffer, dstBuffer);

   uint64_t va = buffer->iova + dstOffset;

   struct tu_cs *cs = cmd->state.pass ? &cmd->draw_cs : &cmd->cs;
   struct tu_cache_state *cache =
      cmd->state.pass ? &cmd->state.renderpass_cache : &cmd->state.cache;

   /* From the Vulkan 1.2.203 spec:
    *
    *    The access scope for buffer marker writes falls under
    *    the VK_ACCESS_TRANSFER_WRITE_BIT, and the pipeline stages for
    *    identifying the synchronization scope must include both pipelineStage
    *    and VK_PIPELINE_STAGE_TRANSFER_BIT.
    *
    * Transfer operations use CCU however here we write via CP.
    * Flush CCU in order to make the results of previous transfer
    * operation visible to CP.
    */
   tu_flush_for_access(cache, TU_ACCESS_NONE, TU_ACCESS_SYSMEM_WRITE);

   /* Flags that only require a top-of-pipe event. DrawIndirect parameters are
    * read by the CP, so the draw indirect stage counts as top-of-pipe too.
    */
   VkPipelineStageFlags2 top_of_pipe_flags =
      VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT |
      VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;

   bool is_top_of_pipe = !(pipelineStage & ~top_of_pipe_flags);

   /* We have to WFI only if we flushed CCU here and are using CP_MEM_WRITE.
    * Otherwise:
    * - We do CP_EVENT_WRITE(RB_DONE_TS) which should wait for flushes;
    * - There was a barrier to synchronize other writes with WriteBufferMarkerAMD
    *   and they had to include our pipelineStage which forces the WFI.
    */
   if (cache->flush_bits && is_top_of_pipe) {
      cache->flush_bits |= TU_CMD_FLAG_WAIT_FOR_IDLE;
   }

   if (cmd->state.pass) {
      tu_emit_cache_flush_renderpass<CHIP>(cmd);
   } else {
      tu_emit_cache_flush<CHIP>(cmd);
   }

   if (is_top_of_pipe) {
      tu_cs_emit_pkt7(cs, CP_MEM_WRITE, 3);
      tu_cs_emit_qw(cs, va); /* ADDR_LO/HI */
      tu_cs_emit(cs, marker);
   } else {
      /* Use a RB_DONE_TS event to wait for everything to complete. */
      if (CHIP == A6XX) {
         tu_cs_emit_pkt7(cs, CP_EVENT_WRITE, 4);
         tu_cs_emit(cs, CP_EVENT_WRITE_0_EVENT(RB_DONE_TS));
      } else {
         tu_cs_emit_pkt7(cs, CP_EVENT_WRITE7, 4);
         tu_cs_emit(cs, CP_EVENT_WRITE7_0(.event = RB_DONE_TS,
                                          .write_src = EV_WRITE_USER_32B,
                                          .write_dst = EV_DST_RAM,
                                          .write_enabled = true).value);
      }
      tu_cs_emit_qw(cs, va);
      tu_cs_emit(cs, marker);
   }

   /* Make sure the result of this write is visible to others. */
   tu_flush_for_access(cache, TU_ACCESS_CP_WRITE, TU_ACCESS_NONE);
}
TU_GENX(tu_CmdWriteBufferMarker2AMD);
