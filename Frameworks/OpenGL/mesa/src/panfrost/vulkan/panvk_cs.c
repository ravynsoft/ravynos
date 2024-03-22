/*
 * Copyright (C) 2021 Collabora Ltd.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "compiler/shader_enums.h"
#include "util/macros.h"

#include "pan_desc.h"
#include "pan_pool.h"

#include "panvk_cs.h"
#include "panvk_private.h"

/*
 * Upload the viewport scale. Defined as (px/2, py/2, pz) at the start of
 * section 24.5 ("Controlling the Viewport") of the Vulkan spec. At the end of
 * the section, the spec defines:
 *
 * px = width
 * py = height
 * pz = maxDepth - minDepth
 */
void
panvk_sysval_upload_viewport_scale(const VkViewport *viewport,
                                   union panvk_sysval_vec4 *data)
{
   data->f32[0] = 0.5f * viewport->width;
   data->f32[1] = 0.5f * viewport->height;
   data->f32[2] = (viewport->maxDepth - viewport->minDepth);
}

/*
 * Upload the viewport offset. Defined as (ox, oy, oz) at the start of section
 * 24.5 ("Controlling the Viewport") of the Vulkan spec. At the end of the
 * section, the spec defines:
 *
 * ox = x + width/2
 * oy = y + height/2
 * oz = minDepth
 */
void
panvk_sysval_upload_viewport_offset(const VkViewport *viewport,
                                    union panvk_sysval_vec4 *data)
{
   data->f32[0] = (0.5f * viewport->width) + viewport->x;
   data->f32[1] = (0.5f * viewport->height) + viewport->y;
   data->f32[2] = viewport->minDepth;
}
