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

#ifndef PVR_JOB_TRANSFER_H
#define PVR_JOB_TRANSFER_H

#include <stdint.h>
#include <vulkan/vulkan.h>

struct pvr_sub_cmd_transfer;
struct pvr_transfer_ctx;
struct vk_sync;

/**
 * Destination pixels not covered by any of the destination rectangles but
 * inside the scissor are filled with the clear color.
 */
#define PVR_TRANSFER_CMD_FLAGS_FILL 0x00000800U
/** If using TQ3D, route to fast2d. */
#define PVR_TRANSFER_CMD_FLAGS_FAST2D 0x00200000U
/** Merge a depth or stencil against a depth + stencil texture. */
#define PVR_TRANSFER_CMD_FLAGS_DSMERGE 0x00000200U
/** Valid if doing a DS merge with depth + stencil to depth + stencil. */
#define PVR_TRANSFER_CMD_FLAGS_PICKD 0x00000400U

VkResult pvr_transfer_job_submit(struct pvr_transfer_ctx *ctx,
                                 struct pvr_sub_cmd_transfer *sub_cmd,
                                 struct vk_sync *wait,
                                 struct vk_sync *signal_sync);

#endif /* PVR_JOB_TRANSFER_H */
