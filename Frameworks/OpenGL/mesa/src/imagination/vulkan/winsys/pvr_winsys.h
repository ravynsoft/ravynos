/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Based on radv_radeon_winsys.h which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
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

#ifndef PVR_WINSYS_H
#define PVR_WINSYS_H

#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_defs.h"
#include "pvr_limits.h"
#include "pvr_rogue_fw.h"
#include "pvr_types.h"
#include "util/macros.h"
#include "util/vma.h"
#include "vk_sync.h"
#include "vk_sync_timeline.h"

struct pvr_device_info;
struct pvr_device_runtime_info;

struct pvr_winsys_heaps {
   struct pvr_winsys_heap *general_heap;
   struct pvr_winsys_heap *pds_heap;
   struct pvr_winsys_heap *rgn_hdr_heap;
   struct pvr_winsys_heap *transfer_frag_heap;
   struct pvr_winsys_heap *usc_heap;
   struct pvr_winsys_heap *vis_test_heap;
};

struct pvr_winsys_static_data_offsets {
   uint64_t eot;
   uint64_t fence;
   uint64_t vdm_sync;
   uint64_t yuv_csc;
};

struct pvr_winsys_heap {
   struct pvr_winsys *ws;

   pvr_dev_addr_t base_addr;
   pvr_dev_addr_t static_data_carveout_addr;

   uint64_t size;
   uint64_t static_data_carveout_size;

   uint32_t page_size;
   uint32_t log2_page_size;

   struct util_vma_heap vma_heap;
   int ref_count;
   pthread_mutex_t lock;

   /* These are the offsets from the base at which static data might be
    * uploaded. Some of these might be invalid since the kernel might not
    * return all of these offsets per each heap as they might not be
    * applicable.
    * You should know which to use beforehand. There should be no need to check
    * whether an offset is valid or invalid.
    */
   struct pvr_winsys_static_data_offsets static_data_offsets;
};

enum pvr_winsys_bo_type {
   PVR_WINSYS_BO_TYPE_GPU = 0,
   PVR_WINSYS_BO_TYPE_DISPLAY = 1,
};

/**
 * \brief Flag passed to #pvr_winsys_ops.buffer_create to indicate that the
 * buffer should be CPU accessible. This is required in order to map the buffer
 * using #pvr_winsys_ops.buffer_map.
 */
#define PVR_WINSYS_BO_FLAG_CPU_ACCESS BITFIELD_BIT(0U)
/**
 * \brief Flag passed to #pvr_winsys_ops.buffer_create to indicate that, when
 * the buffer is mapped to the GPU using #pvr_winsys.vma_map, it should be
 * mapped uncached.
 */
#define PVR_WINSYS_BO_FLAG_GPU_UNCACHED BITFIELD_BIT(1U)
/**
 * \brief Flag passed to #pvr_winsys_ops.buffer_create to indicate that, when
 * the buffer is mapped to the GPU using #pvr_winsys.vma_map, it should only be
 * accessible to the Parameter Manager unit and firmware processor.
 */
#define PVR_WINSYS_BO_FLAG_PM_FW_PROTECT BITFIELD_BIT(2U)

struct pvr_winsys_bo {
   struct pvr_winsys *ws;
   void *map;
   uint64_t size;

   bool is_imported;

#if defined(HAVE_VALGRIND)
   char *vbits;
#endif /* defined(HAVE_VALGRIND) */
};

struct pvr_winsys_vma {
   struct pvr_winsys_heap *heap;

   /* Buffer and offset this vma is bound to. */
   struct pvr_winsys_bo *bo;
   VkDeviceSize bo_offset;

   pvr_dev_addr_t dev_addr;
   uint64_t size;
   uint64_t mapped_size;
};

struct pvr_winsys_free_list {
   struct pvr_winsys *ws;
};

struct pvr_winsys_rt_dataset_create_info {
   /* Local freelist */
   struct pvr_winsys_free_list *local_free_list;

   uint32_t width;
   uint32_t height;
   uint32_t samples;
   uint16_t layers;

   /* ISP register values */
   uint32_t isp_merge_lower_x;
   uint32_t isp_merge_lower_y;
   uint32_t isp_merge_scale_x;
   uint32_t isp_merge_scale_y;
   uint32_t isp_merge_upper_x;
   uint32_t isp_merge_upper_y;

   /* Allocations and associated information */
   pvr_dev_addr_t vheap_table_dev_addr;
   pvr_dev_addr_t rtc_dev_addr;

   pvr_dev_addr_t tpc_dev_addr;
   uint32_t tpc_stride;
   uint32_t tpc_size;

   struct {
      pvr_dev_addr_t pm_mlist_dev_addr;
      pvr_dev_addr_t macrotile_array_dev_addr;
      pvr_dev_addr_t rgn_header_dev_addr;
   } rt_datas[ROGUE_NUM_RTDATAS];
   uint64_t rgn_header_size;
};

struct pvr_winsys_rt_dataset {
   struct pvr_winsys *ws;
};

enum pvr_winsys_ctx_priority {
   PVR_WINSYS_CTX_PRIORITY_LOW,
   PVR_WINSYS_CTX_PRIORITY_MEDIUM,
   PVR_WINSYS_CTX_PRIORITY_HIGH,
};

struct pvr_winsys_render_ctx_create_info {
   enum pvr_winsys_ctx_priority priority;
   pvr_dev_addr_t vdm_callstack_addr;

   struct pvr_winsys_render_ctx_static_state {
      uint64_t vdm_ctx_state_base_addr;
      uint64_t geom_ctx_state_base_addr;

      struct {
         uint64_t vdm_ctx_store_task0;
         uint32_t vdm_ctx_store_task1;
         uint64_t vdm_ctx_store_task2;

         uint64_t vdm_ctx_resume_task0;
         uint32_t vdm_ctx_resume_task1;
         uint64_t vdm_ctx_resume_task2;
      } geom_state[2];
   } static_state;
};

struct pvr_winsys_render_ctx {
   struct pvr_winsys *ws;
};

struct pvr_winsys_compute_ctx_create_info {
   enum pvr_winsys_ctx_priority priority;

   struct pvr_winsys_compute_ctx_static_state {
      uint64_t cdm_ctx_store_pds0;
      uint64_t cdm_ctx_store_pds0_b;
      uint32_t cdm_ctx_store_pds1;

      uint64_t cdm_ctx_terminate_pds;
      uint32_t cdm_ctx_terminate_pds1;

      uint64_t cdm_ctx_resume_pds0;
      uint64_t cdm_ctx_resume_pds0_b;
   } static_state;
};

struct pvr_winsys_compute_ctx {
   struct pvr_winsys *ws;
};

struct pvr_winsys_transfer_ctx_create_info {
   enum pvr_winsys_ctx_priority priority;
};

struct pvr_winsys_transfer_ctx {
   struct pvr_winsys *ws;
};

#define PVR_TRANSFER_MAX_PREPARES_PER_SUBMIT 16U
#define PVR_TRANSFER_MAX_RENDER_TARGETS 3U

struct pvr_winsys_transfer_regs {
   uint32_t event_pixel_pds_code;
   uint32_t event_pixel_pds_data;
   uint32_t event_pixel_pds_info;
   uint32_t frag_screen;
   uint32_t isp_aa;
   uint32_t isp_bgobjvals;
   uint32_t isp_ctl;
   uint64_t isp_mtile_base;
   uint32_t isp_mtile_size;
   uint32_t isp_render;
   uint32_t isp_render_origin;
   uint32_t isp_rgn;
   uint64_t pbe_wordx_mrty[PVR_TRANSFER_MAX_RENDER_TARGETS *
                           ROGUE_NUM_PBESTATE_REG_WORDS];
   uint64_t pds_bgnd0_base;
   uint64_t pds_bgnd1_base;
   uint64_t pds_bgnd3_sizeinfo;
   uint32_t usc_clear_register0;
   uint32_t usc_clear_register1;
   uint32_t usc_clear_register2;
   uint32_t usc_clear_register3;
   uint32_t usc_pixel_output_ctrl;
};

struct pvr_winsys_transfer_cmd {
   /* Firmware stream buffer. This is the maximum possible size taking into
    * consideration all HW features, quirks and enhancements.
    */
   uint8_t fw_stream[172];
   uint32_t fw_stream_len;

   struct pvr_winsys_transfer_cmd_flags {
      bool use_single_core : 1;
   } flags;
};

struct pvr_winsys_transfer_submit_info {
   uint32_t frame_num;
   uint32_t job_num;

   struct vk_sync *wait;

   uint32_t cmd_count;
   struct pvr_winsys_transfer_cmd cmds[PVR_TRANSFER_MAX_PREPARES_PER_SUBMIT];
};

struct pvr_winsys_compute_submit_info {
   uint32_t frame_num;
   uint32_t job_num;

   struct vk_sync *wait;

   /* Firmware stream buffer. This is the maximum possible size taking into
    * consideration all HW features, quirks and enhancements.
    */
   uint8_t fw_stream[100];
   uint32_t fw_stream_len;

   struct pvr_winsys_compute_submit_flags {
      bool prevent_all_overlap : 1;
      bool use_single_core : 1;
   } flags;
};

struct pvr_winsys_render_submit_info {
   struct pvr_winsys_rt_dataset *rt_dataset;
   uint8_t rt_data_idx;

   uint32_t frame_num;
   uint32_t job_num;
   bool has_fragment_job;

   struct pvr_winsys_geometry_state {
      /* Firmware stream buffer. This is the maximum possible size taking into
       * consideration all HW features, quirks and enhancements.
       */
      uint8_t fw_stream[64];
      uint32_t fw_stream_len;

      struct pvr_winsys_geometry_state_flags {
         bool is_first_geometry : 1;
         bool is_last_geometry : 1;
         bool use_single_core : 1;
      } flags;

      struct vk_sync *wait;
   } geometry;

   struct pvr_winsys_fragment_state {
      /* Firmware stream buffer. This is the maximum possible size taking into
       * consideration all HW features, quirks and enhancements.
       */
      uint8_t fw_stream[440];
      uint32_t fw_stream_len;

      struct pvr_winsys_fragment_state_flags {
         bool has_depth_buffer : 1;
         bool has_stencil_buffer : 1;
         bool prevent_cdm_overlap : 1;
         bool use_single_core : 1;
         bool get_vis_results : 1;
         bool has_spm_scratch_buffer : 1;
      } flags;

      struct vk_sync *wait;
   } fragment, fragment_pr;
};

struct pvr_winsys_ops {
   void (*destroy)(struct pvr_winsys *ws);
   VkResult (*device_info_init)(struct pvr_winsys *ws,
                                struct pvr_device_info *dev_info,
                                struct pvr_device_runtime_info *runtime_info);
   void (*get_heaps_info)(struct pvr_winsys *ws,
                          struct pvr_winsys_heaps *heaps);

   VkResult (*buffer_create)(struct pvr_winsys *ws,
                             uint64_t size,
                             uint64_t alignment,
                             enum pvr_winsys_bo_type type,
                             uint32_t flags,
                             struct pvr_winsys_bo **const bo_out);
   VkResult (*buffer_create_from_fd)(struct pvr_winsys *ws,
                                     int fd,
                                     struct pvr_winsys_bo **const bo_out);
   void (*buffer_destroy)(struct pvr_winsys_bo *bo);

   VkResult (*buffer_get_fd)(struct pvr_winsys_bo *bo, int *const fd_out);

   VkResult (*buffer_map)(struct pvr_winsys_bo *bo);
   void (*buffer_unmap)(struct pvr_winsys_bo *bo);

   VkResult (*heap_alloc)(struct pvr_winsys_heap *heap,
                          uint64_t size,
                          uint64_t alignment,
                          struct pvr_winsys_vma **vma_out);
   void (*heap_free)(struct pvr_winsys_vma *vma);

   VkResult (*vma_map)(struct pvr_winsys_vma *vma,
                       struct pvr_winsys_bo *bo,
                       uint64_t offset,
                       uint64_t size,
                       pvr_dev_addr_t *dev_addr_out);
   void (*vma_unmap)(struct pvr_winsys_vma *vma);

   VkResult (*free_list_create)(
      struct pvr_winsys *ws,
      struct pvr_winsys_vma *free_list_vma,
      uint32_t initial_num_pages,
      uint32_t max_num_pages,
      uint32_t grow_num_pages,
      uint32_t grow_threshold,
      struct pvr_winsys_free_list *parent_free_list,
      struct pvr_winsys_free_list **const free_list_out);
   void (*free_list_destroy)(struct pvr_winsys_free_list *free_list);

   VkResult (*render_target_dataset_create)(
      struct pvr_winsys *ws,
      const struct pvr_winsys_rt_dataset_create_info *create_info,
      const struct pvr_device_info *dev_info,
      struct pvr_winsys_rt_dataset **const rt_dataset_out);
   void (*render_target_dataset_destroy)(
      struct pvr_winsys_rt_dataset *rt_dataset);

   VkResult (*render_ctx_create)(
      struct pvr_winsys *ws,
      struct pvr_winsys_render_ctx_create_info *create_info,
      struct pvr_winsys_render_ctx **const ctx_out);
   void (*render_ctx_destroy)(struct pvr_winsys_render_ctx *ctx);
   VkResult (*render_submit)(
      const struct pvr_winsys_render_ctx *ctx,
      const struct pvr_winsys_render_submit_info *submit_info,
      const struct pvr_device_info *dev_info,
      struct vk_sync *signal_sync_geom,
      struct vk_sync *signal_sync_frag);

   VkResult (*compute_ctx_create)(
      struct pvr_winsys *ws,
      const struct pvr_winsys_compute_ctx_create_info *create_info,
      struct pvr_winsys_compute_ctx **const ctx_out);
   void (*compute_ctx_destroy)(struct pvr_winsys_compute_ctx *ctx);
   VkResult (*compute_submit)(
      const struct pvr_winsys_compute_ctx *ctx,
      const struct pvr_winsys_compute_submit_info *submit_info,
      const struct pvr_device_info *dev_info,
      struct vk_sync *signal_sync);

   VkResult (*transfer_ctx_create)(
      struct pvr_winsys *ws,
      const struct pvr_winsys_transfer_ctx_create_info *create_info,
      struct pvr_winsys_transfer_ctx **const ctx_out);
   void (*transfer_ctx_destroy)(struct pvr_winsys_transfer_ctx *ctx);
   VkResult (*transfer_submit)(
      const struct pvr_winsys_transfer_ctx *ctx,
      const struct pvr_winsys_transfer_submit_info *submit_info,
      const struct pvr_device_info *dev_info,
      struct vk_sync *signal_sync);

   VkResult (*null_job_submit)(struct pvr_winsys *ws,
                               struct vk_sync_wait *waits,
                               uint32_t wait_count,
                               struct vk_sync_signal *signal_sync);
};

struct pvr_winsys {
   uint64_t page_size;
   uint32_t log2_page_size;

   const struct vk_sync_type *sync_types[3];
   struct vk_sync_type syncobj_type;
   struct vk_sync_timeline_type timeline_syncobj_type;

   int render_fd;
   int display_fd;

   const VkAllocationCallbacks *alloc;

   struct {
      bool supports_threaded_submit : 1;
   } features;

   const struct pvr_winsys_ops *ops;
};

void pvr_winsys_destroy(struct pvr_winsys *ws);
VkResult pvr_winsys_create(const char *render_path,
                           const char *display_path,
                           const VkAllocationCallbacks *alloc,
                           struct pvr_winsys **ws_out);

#endif /* PVR_WINSYS_H */
