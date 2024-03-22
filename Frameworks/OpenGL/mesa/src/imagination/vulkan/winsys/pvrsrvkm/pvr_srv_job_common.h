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

#ifndef PVR_SRV_JOB_COMMON_H
#define PVR_SRV_JOB_COMMON_H

#include <stdint.h>

#include "pvr_srv_bridge.h"
#include "pvr_winsys.h"
#include "util/macros.h"

#include <vulkan/vulkan_core.h>

static inline uint32_t
pvr_srv_from_winsys_priority(enum pvr_winsys_ctx_priority priority)
{
   switch (priority) {
   case PVR_WINSYS_CTX_PRIORITY_HIGH:
      return RGX_CONTEXT_PRIORITY_HIGH;
   case PVR_WINSYS_CTX_PRIORITY_MEDIUM:
      return RGX_CONTEXT_PRIORITY_MEDIUM;
   case PVR_WINSYS_CTX_PRIORITY_LOW:
      return RGX_CONTEXT_PRIORITY_LOW;
   default:
      unreachable("Invalid winsys context priority.");
   }
}

VkResult pvr_srv_create_timeline(int render_fd, int *const fd_out);

#endif /* PVR_SRV_JOB_COMMON_H */
