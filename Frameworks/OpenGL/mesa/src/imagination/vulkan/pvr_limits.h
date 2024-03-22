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

/**
 * Constants for VkPhysicalDeviceLimits.
 */

#ifndef PVR_LIMITS_H
#define PVR_LIMITS_H

#include "hwdef/rogue_hw_defs.h"
#include "pvr_device_info.h"
#include "util/u_math.h"

#define PVR_MAX_COLOR_ATTACHMENTS PVR_NUM_PBE_EMIT_REGS
#define PVR_MAX_QUEUES 2U
#define PVR_MAX_VIEWPORTS 1U
#define PVR_MAX_NEG_OFFSCREEN_OFFSET 4096U

#define PVR_MAX_PUSH_CONSTANTS_SIZE 256U

#define PVR_MAX_TEXTURE_EXTENT_Z \
   (PVRX(TEXSTATE_IMAGE_WORD1_DEPTH_MAX_SIZE) + 1U)

#define PVR_MAX_ARRAY_LAYERS (PVRX(TEXSTATE_IMAGE_WORD1_DEPTH_MAX_SIZE) + 1U)

#define PVR_MAX_DESCRIPTOR_SETS 4U
#define PVR_MAX_DESCRIPTOR_SET_UNIFORM_DYNAMIC_BUFFERS 8U
#define PVR_MAX_DESCRIPTOR_SET_STORAGE_DYNAMIC_BUFFERS 8U

#define PVR_MAX_FRAMEBUFFER_LAYERS PVR_MAX_ARRAY_LAYERS

/* The limit is somewhat arbitrary, it just translates into more pds code
 * and larger arrays, 32 appears to be the popular (and highest choice) across
 * other implementations.
 */
#define PVR_MAX_VERTEX_INPUT_BINDINGS 16U

/* We need one RenderTarget per supported MSAA mode as each render target
 * contains state that is dependent on the sample count of the render that is
 * rendering to it.
 *
 * As we do not know the sample count until we know the renderpass framebuffer
 * combination being used, we create one per supported sample mode.
 */
#define PVR_RENDER_TARGETS_PER_FRAMEBUFFER(dev_info)                         \
   ({                                                                        \
      uint32_t __ret = PVR_GET_FEATURE_VALUE(dev_info, max_multisample, 4U); \
      util_logbase2(__ret) + 1;                                              \
   })

#endif
