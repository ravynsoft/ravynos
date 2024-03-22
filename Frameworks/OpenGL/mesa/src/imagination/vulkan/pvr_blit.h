/*
 * Copyright © 2023 Imagination Technologies Ltd.
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

#ifndef PVR_BLIT_H
#define PVR_BLIT_H

#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "pvr_types.h"

struct pvr_cmd_buffer;
struct pvr_device;
struct pvr_image;
struct pvr_transfer_cmd;
struct pvr_transfer_cmd_surface;

VkFormat pvr_get_raw_copy_format(VkFormat format);

VkResult
pvr_copy_or_resolve_color_image_region(struct pvr_cmd_buffer *cmd_buffer,
                                       const struct pvr_image *src,
                                       const struct pvr_image *dst,
                                       const VkImageCopy2 *region);

VkResult pvr_copy_buffer_to_image_region(struct pvr_cmd_buffer *cmd_buffer,
                                         pvr_dev_addr_t buffer_dev_addr,
                                         const struct pvr_image *image,
                                         const VkBufferImageCopy2 *region);

VkResult
pvr_copy_buffer_to_image_region_format(struct pvr_cmd_buffer *cmd_buffer,
                                       pvr_dev_addr_t buffer_dev_addr,
                                       const struct pvr_image *image,
                                       const VkBufferImageCopy2 *region,
                                       VkFormat src_format,
                                       VkFormat dst_format,
                                       uint32_t flags);

VkResult pvr_copy_image_to_buffer_region(struct pvr_cmd_buffer *cmd_buffer,
                                         const struct pvr_image *image,
                                         pvr_dev_addr_t buffer_dev_addr,
                                         const VkBufferImageCopy2 *region);

VkResult
pvr_copy_image_to_buffer_region_format(struct pvr_cmd_buffer *cmd_buffer,
                                       const struct pvr_image *image,
                                       pvr_dev_addr_t buffer_dev_addr,
                                       const VkBufferImageCopy2 *region,
                                       VkFormat src_format,
                                       VkFormat dst_format);

/* TODO: Rename this? */
void pvr_clear_attachments_render_init(struct pvr_cmd_buffer *cmd_buffer,
                                       const VkClearAttachment *attachment,
                                       const VkClearRect *rect);

#endif /* PVR_BLIT_H */
