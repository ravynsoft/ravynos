/*
 * Copyright © 2023 Intel Corporation
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
#include "xe/intel_gem.h"

#include "drm-uapi/xe_drm.h"

#include "common/intel_gem.h"
#include "common/xe/intel_engine.h"

bool
xe_gem_read_render_timestamp(int fd, uint64_t *value)
{
   UNUSED uint64_t cpu;

   return xe_gem_read_correlate_cpu_gpu_timestamp(fd, INTEL_ENGINE_CLASS_RENDER,
                                                  0, CLOCK_MONOTONIC, &cpu,
                                                  value, NULL);
}

bool
xe_gem_can_render_on_fd(int fd)
{
   struct drm_xe_device_query query = {
      .query = DRM_XE_DEVICE_QUERY_ENGINES,
   };
   return intel_ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query) == 0;
}

bool
xe_gem_read_correlate_cpu_gpu_timestamp(int fd,
                                        enum intel_engine_class engine_class,
                                        uint16_t engine_instance,
                                        clockid_t cpu_clock_id,
                                        uint64_t *cpu_timestamp,
                                        uint64_t *gpu_timestamp,
                                        uint64_t *cpu_delta)
{
   struct drm_xe_query_engine_cycles engine_cycles = {};
   struct drm_xe_device_query query = {
      .query = DRM_XE_DEVICE_QUERY_ENGINE_CYCLES,
      .size = sizeof(engine_cycles),
      .data = (uintptr_t)&engine_cycles,
   };

   switch (cpu_clock_id) {
   case CLOCK_MONOTONIC:
#ifdef CLOCK_MONOTONIC_RAW
   case CLOCK_MONOTONIC_RAW:
#endif
   case CLOCK_REALTIME:
#ifdef CLOCK_BOOTTIME
   case CLOCK_BOOTTIME:
#endif
#ifdef CLOCK_TAI
   case CLOCK_TAI:
#endif
      break;
   default:
      return false;
   }

   engine_cycles.eci.engine_class = intel_engine_class_to_xe(engine_class);
   engine_cycles.eci.engine_instance = engine_instance;
   engine_cycles.eci.gt_id = 0;
   engine_cycles.clockid = cpu_clock_id;

   if (intel_ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query))
         return false;

   *cpu_timestamp = engine_cycles.cpu_timestamp;
   *gpu_timestamp = engine_cycles.engine_cycles;
   if (cpu_delta)
      *cpu_delta = engine_cycles.cpu_delta;

   return true;
}
