/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __PAN_INDIRECT_DISPATCH_SHADERS_H__
#define __PAN_INDIRECT_DISPATCH_SHADERS_H__

#include "genxml/gen_macros.h"
#include "pan_jc.h"

struct pan_device;
struct pan_jc;
struct pan_pool;

struct pan_indirect_dispatch_info {
   mali_ptr job;
   mali_ptr indirect_dim;
   mali_ptr num_wg_sysval[3];
} PACKED;

unsigned GENX(pan_indirect_dispatch_emit)(
   struct pan_pool *pool, struct pan_jc *jc,
   const struct pan_indirect_dispatch_info *dispatch_info);

void GENX(pan_indirect_dispatch_cleanup)(struct panfrost_device *dev);

#endif
