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

#ifndef PVR_SRV_JOB_RENDER_H
#define PVR_SRV_JOB_RENDER_H

#include <stdint.h>
#include <vulkan/vulkan.h>

struct pvr_device_info;
struct pvr_winsys;
struct pvr_winsys_free_list;
struct pvr_winsys_render_ctx;
struct pvr_winsys_render_ctx_create_info;
struct pvr_winsys_render_submit_info;
struct pvr_winsys_rt_dataset;
struct pvr_winsys_rt_dataset_create_info;
struct pvr_winsys_vma;
struct vk_sync;

/*******************************************
   Function prototypes
 *******************************************/

VkResult pvr_srv_winsys_free_list_create(
   struct pvr_winsys *ws,
   struct pvr_winsys_vma *free_list_vma,
   uint32_t initial_num_pages,
   uint32_t max_num_pages,
   uint32_t grow_num_pages,
   uint32_t grow_threshold,
   struct pvr_winsys_free_list *parent_free_list,
   struct pvr_winsys_free_list **const free_list_out);
void pvr_srv_winsys_free_list_destroy(struct pvr_winsys_free_list *free_list);

VkResult pvr_srv_render_target_dataset_create(
   struct pvr_winsys *ws,
   const struct pvr_winsys_rt_dataset_create_info *create_info,
   const struct pvr_device_info *dev_info,
   struct pvr_winsys_rt_dataset **const rt_dataset_out);
void pvr_srv_render_target_dataset_destroy(
   struct pvr_winsys_rt_dataset *rt_dataset);

VkResult pvr_srv_winsys_render_ctx_create(
   struct pvr_winsys *ws,
   struct pvr_winsys_render_ctx_create_info *create_info,
   struct pvr_winsys_render_ctx **const ctx_out);
void pvr_srv_winsys_render_ctx_destroy(struct pvr_winsys_render_ctx *ctx);

VkResult pvr_srv_winsys_render_submit(
   const struct pvr_winsys_render_ctx *ctx,
   const struct pvr_winsys_render_submit_info *submit_info,
   const struct pvr_device_info *dev_info,
   struct vk_sync *signal_sync_geom,
   struct vk_sync *signal_sync_frag);

#endif /* PVR_SRV_JOB_RENDER_H */
