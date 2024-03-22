/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_JOB_RENDER_H
#define PVR_JOB_RENDER_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_defs.h"
#include "pvr_csb.h"
#include "pvr_limits.h"
#include "pvr_types.h"

struct pvr_device;
struct pvr_device_info;
struct pvr_free_list;
struct pvr_render_ctx;
struct pvr_rt_dataset;
struct vk_sync;

/* Macrotile information. */
struct pvr_rt_mtile_info {
   uint32_t tile_size_x;
   uint32_t tile_size_y;

   uint32_t num_tiles_x;
   uint32_t num_tiles_y;

   uint32_t tiles_per_mtile_x;
   uint32_t tiles_per_mtile_y;

   uint32_t x_tile_max;
   uint32_t y_tile_max;

   uint32_t mtiles_x;
   uint32_t mtiles_y;

   uint32_t mtile_x1;
   uint32_t mtile_y1;
   uint32_t mtile_x2;
   uint32_t mtile_y2;
   uint32_t mtile_x3;
   uint32_t mtile_y3;
};

/* FIXME: Turn 'struct pvr_sub_cmd' into 'struct pvr_job' and change 'struct
 * pvr_render_job' to subclass it? This is approximately what v3dv does
 * (although it doesn't subclass).
 */
struct pvr_render_job {
   struct pvr_rt_dataset *rt_dataset;

   struct {
      bool run_frag : 1;
      bool geometry_terminate : 1;
      bool frag_uses_atomic_ops : 1;
      bool disable_compute_overlap : 1;
      bool enable_bg_tag : 1;
      bool process_empty_tiles : 1;
      bool get_vis_results : 1;
      bool has_depth_attachment : 1;
      bool has_stencil_attachment : 1;
      bool requires_spm_scratch_buffer : 1;
   };

   uint32_t pds_pixel_event_data_offset;
   uint32_t pr_pds_pixel_event_data_offset;

   pvr_dev_addr_t ctrl_stream_addr;

   pvr_dev_addr_t depth_bias_table_addr;
   pvr_dev_addr_t scissor_table_addr;

   /* Unless VK_KHR_dynamic_rendering or core 1.3 is supported, Vulkan does not
    * allow for separate depth and stencil attachments. We don't bother storing
    * separate parameters for them here (yet). If both has_depth_attachment and
    * has_stencil_attachment are false, the contents are undefined.
    */
   struct pvr_ds_attachment {
      struct {
         bool d : 1;
         bool s : 1;
      } load, store;

      pvr_dev_addr_t addr;
      uint32_t stride;
      uint32_t height;
      VkExtent2D physical_extent;
      uint32_t layer_size;
      enum PVRX(CR_ZLS_FORMAT_TYPE) zls_format;
      /* FIXME: This should be of type 'enum pvr_memlayout', but this is defined
       * in pvr_private.h, which causes a circular include dependency. For now,
       * treat it as a uint32_t. A couple of ways to possibly fix this:
       *
       *   1. Merge the contents of this header file into pvr_private.h.
       *   2. Move 'enum pvr_memlayout' into it a new header that can be
       *      included by both this header and pvr_private.h.
       */
      uint32_t memlayout;

      /* TODO: Is this really necessary? Maybe we can extract all useful
       * information and drop this member. */
      const struct pvr_image_view *iview;

      bool has_alignment_transfers;
   } ds;

   VkClearDepthStencilValue ds_clear_value;

   uint32_t samples;

   uint32_t pixel_output_width;

   uint8_t max_shared_registers;

   /* Upper limit for tiles in flight, '0' means use default limit based
    * on partition store.
    */
   uint32_t max_tiles_in_flight;

   static_assert(pvr_cmd_length(PBESTATE_REG_WORD0) == 2,
                 "PBESTATE_REG_WORD0 cannot be stored in uint64_t");
   static_assert(pvr_cmd_length(PBESTATE_REG_WORD1) == 2,
                 "PBESTATE_REG_WORD1 cannot be stored in uint64_t");
   static_assert(ROGUE_NUM_PBESTATE_REG_WORDS >= 2,
                 "Cannot store both PBESTATE_REG_WORD{0,1}");
   uint64_t pbe_reg_words[PVR_MAX_COLOR_ATTACHMENTS]
                         [ROGUE_NUM_PBESTATE_REG_WORDS];
   uint64_t pr_pbe_reg_words[PVR_MAX_COLOR_ATTACHMENTS]
                            [ROGUE_NUM_PBESTATE_REG_WORDS];

   static_assert(pvr_cmd_length(CR_PDS_BGRND0_BASE) == 2,
                 "CR_PDS_BGRND0_BASE cannot be stored in uint64_t");
   static_assert(pvr_cmd_length(CR_PDS_BGRND1_BASE) == 2,
                 "CR_PDS_BGRND1_BASE cannot be stored in uint64_t");
   static_assert(pvr_cmd_length(CR_PDS_BGRND3_SIZEINFO) == 2,
                 "CR_PDS_BGRND3_SIZEINFO cannot be stored in uint64_t");
   static_assert(ROGUE_NUM_CR_PDS_BGRND_WORDS == 3,
                 "Cannot store all CR_PDS_BGRND words");
   uint64_t pds_bgnd_reg_values[ROGUE_NUM_CR_PDS_BGRND_WORDS];
   uint64_t pds_pr_bgnd_reg_values[ROGUE_NUM_CR_PDS_BGRND_WORDS];
};

void pvr_rt_mtile_info_init(const struct pvr_device_info *dev_info,
                            struct pvr_rt_mtile_info *info,
                            uint32_t width,
                            uint32_t height,
                            uint32_t samples);

VkResult pvr_free_list_create(struct pvr_device *device,
                              uint32_t initial_size,
                              uint32_t max_size,
                              uint32_t grow_size,
                              uint32_t grow_threshold,
                              struct pvr_free_list *parent_free_list,
                              struct pvr_free_list **const free_list_out);
void pvr_free_list_destroy(struct pvr_free_list *free_list);

VkResult
pvr_render_target_dataset_create(struct pvr_device *device,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t samples,
                                 uint32_t layers,
                                 struct pvr_rt_dataset **const rt_dataset_out);
void pvr_render_target_dataset_destroy(struct pvr_rt_dataset *dataset);

VkResult pvr_render_job_submit(struct pvr_render_ctx *ctx,
                               struct pvr_render_job *job,
                               struct vk_sync *wait_geom,
                               struct vk_sync *wait_frag,
                               struct vk_sync *signal_sync_geom,
                               struct vk_sync *signal_sync_frag);

#endif /* PVR_JOB_RENDER_H */
