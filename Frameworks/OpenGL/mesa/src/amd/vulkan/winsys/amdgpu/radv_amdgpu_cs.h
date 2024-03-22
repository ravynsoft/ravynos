/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based on amdgpu winsys.
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
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

#ifndef RADV_AMDGPU_CS_H
#define RADV_AMDGPU_CS_H

#include <amdgpu.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "radv_amdgpu_winsys.h"
#include "radv_radeon_winsys.h"

enum { MAX_RINGS_PER_TYPE = 8 };

struct radv_amdgpu_fence {
   struct amdgpu_cs_fence fence;
};

struct radv_amdgpu_ctx {
   struct radv_amdgpu_winsys *ws;
   amdgpu_context_handle ctx;
   struct radv_amdgpu_fence last_submission[AMDGPU_HW_IP_NUM + 1][MAX_RINGS_PER_TYPE];

   struct radeon_winsys_bo *fence_bo;

   uint32_t queue_syncobj[AMDGPU_HW_IP_NUM + 1][MAX_RINGS_PER_TYPE];
   bool queue_syncobj_wait[AMDGPU_HW_IP_NUM + 1][MAX_RINGS_PER_TYPE];
};

static inline struct radv_amdgpu_ctx *
radv_amdgpu_ctx(struct radeon_winsys_ctx *base)
{
   return (struct radv_amdgpu_ctx *)base;
}

void radv_amdgpu_cs_init_functions(struct radv_amdgpu_winsys *ws);

#endif /* RADV_AMDGPU_CS_H */
