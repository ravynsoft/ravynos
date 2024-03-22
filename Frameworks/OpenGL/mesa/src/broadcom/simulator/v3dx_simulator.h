/*
 * Copyright © 2019 Raspberry Pi Ltd
 * Copyright © 2014-2017 Broadcom
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

/* This file generates the per-v3d-version function prototypes.  It must only
 * be included from v3d_simulator.h.
 */

struct v3d_hw;
struct drm_v3d_get_param;
struct drm_v3d_submit_cl;
struct drm_v3d_submit_tfu;
struct drm_v3d_submit_csd;

void v3dX(simulator_init_regs)(struct v3d_hw *v3d);
int v3dX(simulator_get_param_ioctl)(struct v3d_hw *v3d,
                                    struct drm_v3d_get_param *args);
void v3dX(simulator_submit_cl_ioctl)(struct v3d_hw *v3d,
                                     struct drm_v3d_submit_cl *args,
                                     uint32_t gmp_offset);
int v3dX(simulator_submit_tfu_ioctl)(struct v3d_hw *v3d,
                                     struct drm_v3d_submit_tfu *args);
int v3dX(simulator_submit_csd_ioctl)(struct v3d_hw *v3d,
                                     struct drm_v3d_submit_csd *args,
                                     uint32_t gmp_offset);
void v3dX(simulator_perfmon_start)(struct v3d_hw *v3d,
                                   uint32_t ncounters,
                                   uint8_t *events);
void v3dX(simulator_perfmon_stop)(struct v3d_hw *v3d,
                                  uint32_t ncounters,
                                  uint64_t *values);
void v3dX(simulator_get_perfcnt_total)(uint32_t *count);
